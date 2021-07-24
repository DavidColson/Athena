#include "Model.h"

#include "Core/Json.h"
#include "Core/Log.h"
#include "Core/Base64.h"
#include "Core/Vec3.h"
#include "AssetDatabase.h"
#include "Core/FileSystem.h"

#include <SDL.h>

namespace An
{
    // Actually owns the data
    struct Buffer
    {
        char* pBytes{ nullptr };
        size_t byteLength{ 0 };
    };

    // Does not actually own the data
    struct BufferView
    {
        // pointer to some place in a buffer
        char* pBuffer{ nullptr };
        size_t length{ 0 };

        enum Target
        {
            Array,
            ElementArray
        };
        Target target;    
    };

    struct Accessor
    {
        // pointer to some place in a buffer view
        char* pBuffer{ nullptr };
        int count{ 0 };
        enum ComponentType
        {
            Byte,
            UByte,
            Short,
            UShort,
            UInt,
            Float
        };
        ComponentType componentType;

        enum Type
        {
            Scalar,
            Vec2,
            Vec3,
            Vec4,
            Mat2,
            Mat3,
            Mat4
        };
        Type type;
    };

    void Model::Load(Path path, AssetHandle handleForThis)
    {
        // Load file
        eastl::string file = FileSys::ReadWholeFile(path);

        // Parse json
        JsonValue parsed = ParseJsonFile(file);

        // Check for asset label
        bool validGltf = parsed["asset"]["version"].ToString() == "2.0";
        if (!validGltf)
        {
            Log::Crit("Failed to load Gltf file, invalid contents: %s", path);
        }

        eastl::vector<Buffer> rawDataBuffers;
        JsonValue& jsonBuffers = parsed["buffers"];
        for (int i = 0; i < jsonBuffers.Count(); i++)
        {
            Buffer buf;
            buf.byteLength = jsonBuffers[i]["byteLength"].ToInt();
            buf.pBytes = new char[buf.byteLength];
            
            eastl::string encodedBuffer = jsonBuffers[i]["uri"].ToString().substr(37);
            memcpy(buf.pBytes, DecodeBase64(encodedBuffer).data(), buf.byteLength);

            rawDataBuffers.push_back(buf);
        }

        eastl::vector<BufferView> bufferViews;
        JsonValue& jsonBufferViews = parsed["bufferViews"];

        for (int i = 0; i < jsonBufferViews.Count(); i++)
        {
            BufferView view;

            int bufIndex = jsonBufferViews[i]["buffer"].ToInt();
            view.pBuffer = rawDataBuffers[bufIndex].pBytes + jsonBufferViews[i]["byteOffset"].ToInt(); //@Incomplete, byte offset could not be provided, in which case we assume 0

            view.length = jsonBufferViews[i]["byteLength"].ToInt();

            // @Incomplete, target may not be provided
            int target = jsonBufferViews[i]["target"].ToInt();
            if (target == 34963)
                view.target = BufferView::ElementArray;
            else if (target = 34962)
                view.target = BufferView::Array;
            bufferViews.push_back(view);
        }

        eastl::vector<Accessor> accessors;
        JsonValue& jsonAccessors = parsed["accessors"];
        accessors.reserve(jsonAccessors.Count());

        for (int i = 0; i < jsonAccessors.Count(); i++)
        {
            Accessor acc;
            JsonValue& jsonAcc = jsonAccessors[i];

            int idx = jsonAcc["bufferView"].ToInt();
            acc.pBuffer = bufferViews[idx].pBuffer + jsonAcc["byteOffset"].ToInt();
            
            acc.count = jsonAcc["count"].ToInt();

            int compType = jsonAcc["componentType"].ToInt();
            switch (compType)
            {
            case 5120: acc.componentType = Accessor::Byte; break;
            case 5121: acc.componentType = Accessor::UByte; break;
            case 5122: acc.componentType = Accessor::Short; break;
            case 5123: acc.componentType = Accessor::UShort; break;
            case 5125: acc.componentType = Accessor::UInt; break;
            case 5126: acc.componentType = Accessor::Float; break;
            default: break;
            }

            eastl::string type = jsonAcc["type"].ToString();
            if (type == "SCALAR") acc.type = Accessor::Scalar;
            else if (type == "VEC2") acc.type = Accessor::Vec2;
            else if (type == "VEC3") acc.type = Accessor::Vec3;
            else if (type == "VEC4") acc.type = Accessor::Vec4;
            else if (type == "MAT2") acc.type = Accessor::Mat2;
            else if (type == "MAT3") acc.type = Accessor::Mat3;
            else if (type == "MAT4") acc.type = Accessor::Mat4;

            accessors.push_back(acc);
        }
        
        // This is icky. AssetDB stores subasset ptrs, so we cannot resize the array or it'll move the subassets. 
        // Maybe better to let asset DB hold the memory for them?
        meshes.reserve(parsed["meshes"].Count());

        for (int i = 0; i < parsed["meshes"].Count(); i++)
        {
            if (i == 13)
                Log::Debug("Hello");

            JsonValue& jsonMesh = parsed["meshes"][i];
            Mesh mesh;
            mesh.m_name = jsonMesh.HasKey("name") ? jsonMesh["name"].ToString() : "";

            for (int j = 0; j < jsonMesh["primitives"].Count(); j++)
            {
                JsonValue& jsonPrimitive = jsonMesh["primitives"][j];
                Primitive prim;

                if (jsonPrimitive.HasKey("mode"))
                {
                    switch (jsonPrimitive["mode"].ToInt())
                    {
                    case 0: prim.m_topologyType = Primitive::TopologyType::PointList; break;
                    case 1: prim.m_topologyType = Primitive::TopologyType::LineList; break;
                    case 3: prim.m_topologyType = Primitive::TopologyType::LineStrip; break;
                    case 4: prim.m_topologyType = Primitive::TopologyType::TriangleList; break;
                    case 5: prim.m_topologyType = Primitive::TopologyType::TriangleStrip; break;
                    default: Log::Crit("Unsupported topology type given in file %s", path);
                        break;
                    }
                }

                int nVerts = accessors[jsonPrimitive["attributes"]["POSITION"].ToInt()].count;

                JsonValue& jsonAttr = jsonPrimitive["attributes"];
                Vec3f* vertPositionBuffer = (Vec3f*)accessors[jsonAttr["POSITION"].ToInt()].pBuffer;
                prim.m_vertices = eastl::vector<Vec3f>(vertPositionBuffer, vertPositionBuffer + nVerts);
                
                Vec3f* vertNormBuffer = jsonAttr.HasKey("NORMAL") ? (Vec3f*)accessors[jsonAttr["NORMAL"].ToInt()].pBuffer : nullptr;
                prim.m_normals = eastl::vector<Vec3f>(vertNormBuffer, vertNormBuffer + nVerts);

                Vec2f* vertTexCoordBuffer = jsonAttr.HasKey("TEXCOORD_0") ? (Vec2f*)accessors[jsonAttr["TEXCOORD_0"].ToInt()].pBuffer : nullptr;
                prim.m_uv0 = eastl::vector<Vec2f>(vertTexCoordBuffer, vertTexCoordBuffer + nVerts);

                if (jsonAttr.HasKey("COLOR_0"))
                {
                    Vec4f* vertColBuffer = (Vec4f*)accessors[jsonAttr["COLOR_0"].ToInt()].pBuffer;
                    prim.m_colors = eastl::vector<Vec4f>(vertColBuffer, vertColBuffer + nVerts);
                }

                int nIndices = accessors[jsonPrimitive["indices"].ToInt()].count;
                uint16_t* indexBuffer = (uint16_t*)accessors[jsonPrimitive["indices"].ToInt()].pBuffer;
                prim.m_indices = eastl::vector<uint16_t>(indexBuffer, indexBuffer + nIndices);

                prim.RecalcLocalBounds();
                mesh.m_primitives.push_back(eastl::move(prim));
            }
            meshes.push_back(eastl::move(mesh));

            // We'll manually register the mesh asset, since we created it rather than the asset database
            eastl::string meshAssetIdentifier = AssetDB::GetAssetIdentifier(handleForThis);
            meshAssetIdentifier.append_sprintf(":mesh_%i", i);
            meshes.back().Load(meshAssetIdentifier, AssetHandle(meshAssetIdentifier));

            AssetDB::RegisterSubasset(&(meshes.back()), AssetHandle(handleForThis), AssetHandle(meshAssetIdentifier));
        }

        for (int i = 0; i < rawDataBuffers.size(); i++)
        {
            delete rawDataBuffers[i].pBytes;
        }
    }
}