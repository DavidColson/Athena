// Copyright 2020-2021 David Colson. All rights reserved.

#pragma once

#include "AssetDatabase.h"

#include <bgfx/bgfx.h>

namespace An
{
    struct Shader : Asset
    {
        enum Type
        {
            Fragment,
            Vertex
        };

        Shader();
        virtual void Load(Path path, AssetHandle handleForThis) override;
        virtual void Reload(Path path, AssetHandle handleForThis) override;
        virtual ~Shader() override;

        Type m_type;

        bgfx::ShaderHandle m_handle;
    };
}
