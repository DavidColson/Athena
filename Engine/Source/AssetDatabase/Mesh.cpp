// Copyright 2020-2021 David Colson. All rights reserved.

#include "Mesh.h"

namespace An
{
     bgfx::VertexLayout Primitive::s_vertLayout;
    bgfx::VertexLayout Primitive::s_uv0Layout;
    bgfx::VertexLayout Primitive::s_normLayout;
    bgfx::VertexLayout Primitive::s_colLayout;

    // ***********************************************************************

    Primitive::Primitive(const Primitive& copy)
    {
        m_vertices = eastl::vector<Vec3f>(copy.m_vertices);
        m_uv0 = eastl::vector<Vec2f>(copy.m_uv0);
        m_normals = eastl::vector<Vec3f>(copy.m_normals);
        m_colors = eastl::vector<Vec4f>(copy.m_colors);
        m_indices = eastl::vector<uint16_t>(copy.m_indices);
        m_topologyType = copy.m_topologyType;
        m_localBounds = copy.m_localBounds;

        CreateBuffers();
    }

	// ***********************************************************************

    Primitive::Primitive(Primitive&& copy)
    {
        m_vertices = eastl::move(copy.m_vertices);
        m_uv0 = eastl::move(copy.m_uv0);
        m_normals = eastl::move(copy.m_normals);
        m_colors = eastl::move(copy.m_colors);
        m_indices = eastl::move(copy.m_indices);
        m_topologyType = copy.m_topologyType;
        m_localBounds = copy.m_localBounds;

        m_vertexBuffer = copy.m_vertexBuffer;
        m_uv0Buffer = copy.m_uv0Buffer;
        m_normalsBuffer = copy.m_normalsBuffer;
        m_colorBuffer = copy.m_colorBuffer;
        m_indexBuffer = copy.m_indexBuffer;

        copy.m_vertexBuffer = BGFX_INVALID_HANDLE;
        copy.m_uv0Buffer = BGFX_INVALID_HANDLE;
        copy.m_normalsBuffer = BGFX_INVALID_HANDLE;
        copy.m_colorBuffer = BGFX_INVALID_HANDLE;
        copy.m_indexBuffer = BGFX_INVALID_HANDLE;
    }

	// ***********************************************************************

    Primitive& Primitive::operator=(const Primitive& copy)
    {
        bgfx::destroy(m_vertexBuffer);
        bgfx::destroy(m_uv0Buffer);
        bgfx::destroy(m_normalsBuffer);
        bgfx::destroy(m_colorBuffer);
        bgfx::destroy(m_indexBuffer);

        m_vertices = eastl::vector<Vec3f>(copy.m_vertices);
        m_uv0 = eastl::vector<Vec2f>(copy.m_uv0);
        m_normals = eastl::vector<Vec3f>(copy.m_normals);
        m_colors = eastl::vector<Vec4f>(copy.m_colors);
        m_indices = eastl::vector<uint16_t>(copy.m_indices);
        m_topologyType = copy.m_topologyType;
        m_localBounds = copy.m_localBounds;
        CreateBuffers();
        
        return *this;
    }

	// ***********************************************************************

    Primitive& Primitive::operator=(Primitive&& copy)
    {
        m_vertices = eastl::move(copy.m_vertices);
        m_uv0 = eastl::move(copy.m_uv0);
        m_normals = eastl::move(copy.m_normals);
        m_colors = eastl::move(copy.m_colors);
        m_indices = eastl::move(copy.m_indices);
        m_topologyType = copy.m_topologyType;

        m_localBounds = copy.m_localBounds;

        m_vertexBuffer = copy.m_vertexBuffer;
        m_uv0Buffer = copy.m_uv0Buffer;
        m_normalsBuffer = copy.m_normalsBuffer;
        m_colorBuffer = copy.m_colorBuffer;
        m_indexBuffer = copy.m_indexBuffer;

        copy.m_vertexBuffer = BGFX_INVALID_HANDLE;
        copy.m_uv0Buffer = BGFX_INVALID_HANDLE;
        copy.m_normalsBuffer = BGFX_INVALID_HANDLE;
        copy.m_colorBuffer = BGFX_INVALID_HANDLE;
        copy.m_indexBuffer = BGFX_INVALID_HANDLE;

        return *this;
    }

	// ***********************************************************************

    void Primitive::Destroy()
    {
        if (bgfx::isValid(m_vertexBuffer)) 
            bgfx::destroy(m_vertexBuffer);
        if (bgfx::isValid(m_uv0Buffer)) 
            bgfx::destroy(m_uv0Buffer);
        if (bgfx::isValid(m_normalsBuffer)) 
            bgfx::destroy(m_normalsBuffer);
        if (bgfx::isValid(m_colorBuffer)) 
            bgfx::destroy(m_colorBuffer);
        if (bgfx::isValid(m_indexBuffer)) 
            bgfx::destroy(m_indexBuffer);

        m_vertexBuffer = BGFX_INVALID_HANDLE;
        m_uv0Buffer = BGFX_INVALID_HANDLE;
        m_normalsBuffer = BGFX_INVALID_HANDLE;
        m_colorBuffer = BGFX_INVALID_HANDLE;
        m_indexBuffer = BGFX_INVALID_HANDLE;
    }

	// ***********************************************************************

    Primitive::~Primitive()
    {
        Destroy();
    }

	// ***********************************************************************

    void Primitive::InitPrimitiveLayouts()
    {
        s_vertLayout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.end();

		s_colLayout
			.begin()
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
			.end();

        s_normLayout
			.begin()
			.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
			.end();
        
        s_uv0Layout
			.begin()
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.end();
    }

	// ***********************************************************************

    void Primitive::RecalcLocalBounds()
    {
        // Loop through vertices, collecitng the max in each axis.
        for (int i = 0; i < 3; i++)
        {
            Vec3f dir;
            dir[i] = 1.0f;

            float minProj = FLT_MAX; float maxProj = -FLT_MAX;
            int vertMin; int vertMax;
            for (int n = 0; n < (int)m_vertices.size(); n++)
            {
                float projection = Vec3f::Dot(m_vertices[n], dir);
                if (projection < minProj)
                {
                    minProj = projection;
                    vertMin = n;
                }

                if (projection > maxProj)
                {
                    maxProj = projection;
                    vertMax = n;
                }
            }
            m_localBounds.min[i] = m_vertices[vertMin][i];
            m_localBounds.max[i] = m_vertices[vertMax][i];
        }
    }

	// ***********************************************************************

    void Primitive::CreateBuffers()
    {
        if (!m_vertices.empty()) 
        {
	        uint32_t size = uint32_t(sizeof(Vec3f) * m_vertices.size());
            m_vertexBuffer = bgfx::createVertexBuffer(bgfx::makeRef(m_vertices.data(), size), s_vertLayout);
        }

        if (!m_uv0.empty()) 
        {
	        uint32_t size = uint32_t(sizeof(Vec2f) * m_uv0.size());
            m_uv0Buffer = bgfx::createVertexBuffer(bgfx::makeRef(m_uv0.data(), size), s_uv0Layout);
        }

        if (!m_normals.empty()) 
        {
	        uint32_t size = uint32_t(sizeof(Vec3f) * m_normals.size());
            m_normalsBuffer = bgfx::createVertexBuffer(bgfx::makeRef(m_normals.data(), size), s_normLayout);
        }

        if (!m_colors.empty()) 
        {
	        uint32_t size = uint32_t(sizeof(Vec4f) * m_colors.size());
            m_colorBuffer = bgfx::createVertexBuffer(bgfx::makeRef(m_colors.data(), size), s_colLayout);
        }

        if (!m_indices.empty()) 
        {
	        uint32_t size = uint32_t(sizeof(uint16_t) * m_indices.size());
            m_indexBuffer = bgfx::createIndexBuffer(bgfx::makeRef(m_indices.data(), size));
        }
    }
}