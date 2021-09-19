// Copyright 2020-2021 David Colson. All rights reserved.

#pragma once

#include "Core/Path.h"

#include <bgfx/bgfx.h>

namespace An
{
    struct Shader
    {
        enum Type
        {
            Fragment,
            Vertex
        };

        Shader() {}
        Shader(Path path);
        ~Shader();
        
        void Reload(Path path);

        Type m_type;

        bgfx::ShaderHandle m_handle;
    };
}
