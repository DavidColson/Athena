// Copyright 2020-2021 David Colson. All rights reserved.

#include "Image.h"

#include "Core/FileSystem.h"
#include "Core/Log.h"

#include <bimg/decode.h>
#include <bimg/bimg.h>
#include <bx/bx.h>
#include <bx/allocator.h>
#include <bx/error.h>

namespace An
{
    static void ImageFreeCallback(void* _ptr, void* _userData)
    {
        BX_UNUSED(_ptr);
        bimg::ImageContainer* pContainer = (bimg::ImageContainer*)_userData;
        bimg::imageFree(pContainer);
    }

    Image::Image(Path path)
    {
        eastl::string file = FileSys::ReadWholeFile(path);

        static bx::DefaultAllocator allocator;
        bx::Error error;
        bimg::ImageContainer* pContainer = bimg::imageParse(&allocator, file.data(), (uint32_t)file.size(), bimg::TextureFormat::Count, &error);

        if (!bgfx::isTextureValid(0, false, pContainer->m_numLayers, bgfx::TextureFormat::Enum(pContainer->m_format), BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE))
        {
            Log::Crit("Failed to load image %s", path.AsRawString());
            return;
        }

        m_width = pContainer->m_width;
        m_height = pContainer->m_height;
        m_format = bgfx::TextureFormat::Enum(pContainer->m_format);

        const bgfx::Memory* mem = bgfx::makeRef(pContainer->m_data, pContainer->m_size, ImageFreeCallback, pContainer);
        m_gpuHandle = bgfx::createTexture2D((uint16_t)m_width, (uint16_t)m_height, 1 < pContainer->m_numMips, pContainer->m_numLayers, m_format, BGFX_TEXTURE_NONE|BGFX_SAMPLER_NONE, mem);
    }

    Image::~Image()
    {
        bgfx::destroy(m_gpuHandle);
    }
}