// Copyright 2020-2021 David Colson. All rights reserved.

#pragma once

#include "AssetDatabase.h"
#include "Mesh.h"

namespace An
{
    struct Model : Asset
    {
        // TODO: Solution here is to store just the vector of asset handles so ref counting works.
        // Rule is to NEVER store assets directly. Provide an "AllocateAsset" function in assetdb, and it owns all data to assets, so this mesh list moves to there, and we simply store the handle
        eastl::vector<Mesh> meshes;

        virtual void Load(Path path, AssetHandle handleForThis) override;
        ~Model() {}
    };
}