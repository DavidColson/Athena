// Copyright 2020-2021 David Colson. All rights reserved.

#include "AssetDatabase.h"

#include "Core/Log.h"
#include "Text.h"
#include "Core/FileSystem.h"

#include <EASTL/map.h>
#include <EASTL/string.h>

namespace An
{
    namespace
    {
        struct AssetMetaData
        {
            bool IsSubasset() const { return !m_subAssetName.empty(); } 

            enum State
            {
                Loaded,
                Unloaded
            };
            eastl::string m_fullIdentifier;
            eastl::string m_subAssetName;
            Path m_path;
            uint32_t m_refCount{ 0 };
            eastl::vector<AssetHandle> m_subassets;
            State m_state{ State::Unloaded };
            Asset* m_pAssetData{ nullptr };
            uint64_t m_id{ 0 };
        };
        eastl::map<uint64_t, AssetMetaData> assets;

        struct HotReloadingAsset
        {
            uint64_t assetID;
            uint64_t cacheLastModificationTime;
        };
        eastl::vector<HotReloadingAsset> hotReloadWatches;
    }


    // Asset Handle
    // ****************************************************

    uint64_t CalculateFNV(const char* str)
    {
        uint64_t offset = 14695981039346656037u;
        uint64_t prime = 1099511628211u;

        uint64_t hash = offset;
        while (*str != 0)
        {
            hash ^= *str++;
            hash *= prime;
        }
        return hash;
    }

    // ****************************************************

    AssetHandle::AssetHandle(eastl::string identifier)
    {
        id = CalculateFNV(identifier.c_str());
        if (assets.count(id) == 0)
        {
            eastl::string subAssetName;
            size_t subAssetPos  = identifier.find_last_of(":");
            if (subAssetPos != eastl::string::npos)
                subAssetName = identifier.substr(subAssetPos + 1);

            AssetMetaData meta;
            meta.m_path = Path(identifier.substr(0, subAssetPos));
            meta.m_subAssetName = subAssetName;
            meta.m_fullIdentifier = identifier;
            meta.m_id = id;
            assets[id] = meta;
        }
        assets[id].m_refCount += 1;
    }

    // ****************************************************

    AssetHandle::AssetHandle(uint64_t id)
    {
        if (assets.count(id) >= 0)
        {
            assets[id].m_refCount += 1;
        }
    }

    // ****************************************************

    AssetHandle::AssetHandle(const AssetHandle& copy)
    {
        id = copy.id;
        assets[id].m_refCount += 1;
    }

    // ****************************************************

    AssetHandle::AssetHandle(AssetHandle&& move)
    {
        // Don't increment ref counter
        id = move.id;
        move.id = 0;
    }

    // ****************************************************

    AssetHandle& AssetHandle::operator=(const AssetHandle& copy)
    {
        id = copy.id;
        assets[id].m_refCount += 1;
        return *this;
    }

    // ****************************************************

    AssetHandle& AssetHandle::operator=(AssetHandle&& move)
    {
        // Don't increment ref counter
        id = move.id;
        move.id = 0;
        return *this;
    }

    // ****************************************************

    bool AssetHandle::operator==(const AssetHandle& other)
    {
        return id == other.id;
    }

    // ****************************************************

    bool AssetHandle::operator!=(const AssetHandle& other)
    {
        return id != other.id;
    }

    // ****************************************************

    AssetHandle::~AssetHandle()
    {
        if (id != 0)
            assets[id].m_refCount -= 1;
    }



    // Asset Database
    // ****************************

    void LoadAsset(AssetMetaData& asset)
    {
        if (asset.m_state == AssetMetaData::Loaded)
        {
            Log::Warn("Attempting to load an already loaded asset %s", asset.m_path.AsString().c_str());
            return;
        }

        const Path& path = asset.m_path;
        eastl::string fileType = path.Extension().AsString();

        // Create assetdata
        if (fileType == ".txt")
        {
            asset.m_pAssetData = new Text();
        }
        else
        {
            Log::Crit("Attempting to load an unsupported asset type");
            return;
        }

        // Load the asset
        if (FileSys::Exists(asset.m_path))
        {
            asset.m_pAssetData->Load(asset.m_path, AssetHandle(asset.m_fullIdentifier));
        }
        else
        {
            Log::Crit("Unable to load asset %s from engine (%s) or game (%s) resource folders", path.AsRawString(), "Resources/", "Resources/");
            return;
        }

        asset.m_state = AssetMetaData::Loaded;
    }

    // ****************************************************

    void RegisterAssetForHotReload(AssetMetaData& asset)
    {
        //if (Engine::GetConfig().hotReloadingAssetsEnabled)
        if (true && !asset.IsSubasset()) // We don't register subassets for hot reload because their parents will do that for them
        {
            for (const HotReloadingAsset& hot : hotReloadWatches)
            {
                if (hot.assetID == asset.m_id)
                    return;
            }
            // Don't hot reload audio
            // @Improvement actually define an asset type enum or something, this is icky
            if (asset.m_path.Extension() != ".wav")
            {
                HotReloadingAsset hot;
                hot.assetID = asset.m_id; 
                hot.cacheLastModificationTime = FileSys::LastWriteTime(assets[hot.assetID].m_path);
                hotReloadWatches.push_back(hot);
            }
        }
    }

    // ****************************************************

    Asset* AssetDB::GetAssetRaw(AssetHandle handle)
    {
        AssetMetaData& asset = assets[handle.id];

        // TODO: consider the asset type when someone gets an asset, if they request an asset that exists, but it's a different type, gracefully fail
        // TODO: Test if the path actually points to a real file, and error if not
        if (asset.m_state == AssetMetaData::Unloaded)
        {
            if (asset.m_path.IsEmpty())
                return nullptr;

            if (asset.IsSubasset())
            {
                AssetHandle parentAssetHandle(asset.m_path.AsString());
                LoadAsset(assets[parentAssetHandle.id]);
                RegisterAssetForHotReload(assets[parentAssetHandle.id]);
            }
            else
            {
                LoadAsset(asset);
                RegisterAssetForHotReload(asset);
            }

        }

        return assets[handle.id].m_pAssetData;
    }

    // ****************************************************

    eastl::string AssetDB::GetAssetIdentifier(AssetHandle handle)
    {
        if (assets.count(handle.id) == 0)
            return "";

        return assets[handle.id].m_fullIdentifier;
    }

    // ****************************************************

    void AssetDB::FreeAsset(AssetHandle handle)
    {
        AssetMetaData& asset = assets[handle.id];
        if (asset.m_state == AssetMetaData::Unloaded)
            return;

        // Don't actually free subassets, let their parents be freed instead
        if (asset.IsSubasset()) 
            return;

        // Free subassets
        for (AssetHandle subAssetHandle : asset.m_subassets)
        {
            FreeAsset(subAssetHandle);
        }

        delete asset.m_pAssetData;
        asset.m_state = AssetMetaData::Unloaded;
    }

    // ****************************************************

    bool AssetDB::IsSubasset(AssetHandle handle)
    {
        return assets[handle.id].IsSubasset();
    }

    // ****************************************************

    void AssetDB::UpdateHotReloading()
    {
        for (HotReloadingAsset& hot : hotReloadWatches)
        {
            if (hot.cacheLastModificationTime != FileSys::LastWriteTime(assets[hot.assetID].m_path))
            {
                if (assets[hot.assetID].m_refCount == 0)
                    continue; // Don't hot reload an asset no one is referencing

                if (FileSys::IsInUse(assets[hot.assetID].m_path))
                    continue;

                Asset* pAsset = assets[hot.assetID].m_pAssetData;
                if (pAsset->bOverrideReload)
                {
                    pAsset->Reload(assets[hot.assetID].m_path, AssetHandle(hot.assetID));
                }
                else
                {
                    FreeAsset(AssetHandle(hot.assetID));
                    LoadAsset(assets[hot.assetID]);
                }

                hot.cacheLastModificationTime = FileSys::LastWriteTime(assets[hot.assetID].m_path);
                Log::Info("Reloaded asset %s..", assets[hot.assetID].m_path.AsRawString());
            }
        }
    }

    // ****************************************************

    void AssetDB::CollectGarbage()
    {
        for (const eastl::pair<uint64_t, AssetMetaData>& assetMeta : assets)
        {
            if (assetMeta.second.m_refCount == 0)
            {
                FreeAsset(AssetHandle(assetMeta.second.m_fullIdentifier));
            }
        }
    }
}