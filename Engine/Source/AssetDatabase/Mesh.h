// Copyright 2020-2021 David Colson. All rights reserved.

#pragma once

#include "Core/Vec3.h"
#include "Core/Vec4.h"
#include "Core/AABB.h"

#include <bgfx/bgfx.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>

namespace An
{
    struct Primitive
    {
        enum class TopologyType
        {
            TriangleStrip,
            TriangleList,
            LineStrip,
            LineList,
            PointList,
        };

        Primitive() {}
        Primitive(const Primitive& copy);
        Primitive(Primitive&& copy);
        Primitive& operator=(const Primitive& copy);
        Primitive& operator=(Primitive&& copy);
        ~Primitive();
        
        static void InitPrimitiveLayouts();

        void Destroy();
        void RecalcLocalBounds();
        void CreateBuffers();

        eastl::string m_name{"Primitive"};
        TopologyType m_topologyType{TopologyType::TriangleList};
        AABBf m_localBounds;

        eastl::vector<Vec3f> m_vertices;
        eastl::vector<Vec2f> m_uv0;
        eastl::vector<Vec3f> m_normals;
        eastl::vector<Vec4f> m_colors;
        eastl::vector<uint16_t> m_indices{ nullptr };
        bgfx::VertexBufferHandle m_vertexBuffer{ BGFX_INVALID_HANDLE };
        bgfx::VertexBufferHandle m_normalsBuffer{ BGFX_INVALID_HANDLE };
        bgfx::VertexBufferHandle m_uv0Buffer{ BGFX_INVALID_HANDLE };
        bgfx::VertexBufferHandle m_colorBuffer{ BGFX_INVALID_HANDLE };
        bgfx::IndexBufferHandle m_indexBuffer{ BGFX_INVALID_HANDLE };

        static bgfx::VertexLayout s_vertLayout;
        static bgfx::VertexLayout s_normLayout;
        static bgfx::VertexLayout s_uv0Layout;
        static bgfx::VertexLayout s_colLayout;
    };

    struct Mesh
    {
        eastl::string m_name;
        eastl::string m_texture;
        eastl::vector<Primitive> m_primitives;
    };
}