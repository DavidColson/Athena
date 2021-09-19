// Copyright 2020-2021 David Colson. All rights reserved.

#pragma once

#include "Core/Path.h"

#include <bgfx/bgfx.h>


namespace An
{
    struct Image
    {
        Image() {}
        Image(Path path);
        ~Image();

        int m_width;
        int m_height;
        bgfx::TextureFormat::Enum m_format;
        bgfx::TextureHandle m_gpuHandle;
    };
}
