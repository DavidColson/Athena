#pragma once

#include "AssetDatabase.h"

#include <bgfx/bgfx.h>

namespace An
{
    struct Image : Asset
    {
        virtual void Load(Path path, AssetHandle handleForThis) override;

        ~Image();

        int m_width;
        int m_height;
        bgfx::TextureFormat::Enum m_format;
        bgfx::TextureHandle m_gpuHandle;
    };
}
