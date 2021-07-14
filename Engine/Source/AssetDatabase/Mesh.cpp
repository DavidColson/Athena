#include "Mesh.h"

#include "AssetDatabase.h"

namespace An
{
    bgfx::VertexLayout Primitive::s_vertLayout;
    bgfx::VertexLayout Primitive::s_normLayout;
    bgfx::VertexLayout Primitive::s_colLayout;

    void Mesh::Load(Path path, AssetHandle handleForThis)
    {
        // Doesn't actually load anything from disk since this would be a subasset of a model
        // If (AssetDB::IsSubasset(handle))
        // bla bla
        CreateBuffers();
    }

    void Mesh::CreateBuffers()
    {
        for(Primitive& prim : m_primitives)
        {
            prim.CreateBuffers();
        }
    }

    Primitive::Primitive(const Primitive& copy)
    {
        m_vertices = eastl::vector<Vec3f>(copy.m_vertices);
        m_normals = eastl::vector<Vec3f>(copy.m_normals);
        m_colors = eastl::vector<Vec4f>(copy.m_colors);
        m_indices = eastl::vector<uint16_t>(copy.m_indices);
        m_topologyType = copy.m_topologyType;
        m_localBounds = copy.m_localBounds;

        CreateBuffers();
    }

    Primitive::Primitive(Primitive&& copy)
    {
        m_vertices = eastl::move(copy.m_vertices);
        m_normals = eastl::move(copy.m_normals);
        m_colors = eastl::move(copy.m_colors);
        m_indices = eastl::move(copy.m_indices);
        m_topologyType = copy.m_topologyType;
        m_localBounds = copy.m_localBounds;

        m_vertexBuffer = copy.m_vertexBuffer;
        m_normalsBuffer = copy.m_normalsBuffer;
        m_colorBuffer = copy.m_colorBuffer;
        m_indexBuffer = copy.m_indexBuffer;

        copy.m_vertexBuffer = BGFX_INVALID_HANDLE;
        copy.m_normalsBuffer = BGFX_INVALID_HANDLE;
        copy.m_colorBuffer = BGFX_INVALID_HANDLE;
        copy.m_indexBuffer = BGFX_INVALID_HANDLE;
    }

    Primitive& Primitive::operator=(const Primitive& copy)
    {
        bgfx::destroy(m_vertexBuffer);
        bgfx::destroy(m_normalsBuffer);
        bgfx::destroy(m_colorBuffer);
        bgfx::destroy(m_indexBuffer);

        m_vertices = eastl::vector<Vec3f>(copy.m_vertices);
        m_normals = eastl::vector<Vec3f>(copy.m_normals);
        m_colors = eastl::vector<Vec4f>(copy.m_colors);
        m_indices = eastl::vector<uint16_t>(copy.m_indices);
        m_topologyType = copy.m_topologyType;
        m_localBounds = copy.m_localBounds;
        CreateBuffers();
        
        return *this;
    }

    Primitive& Primitive::operator=(Primitive&& copy)
    {
        m_vertices = eastl::move(copy.m_vertices);
        m_normals = eastl::move(copy.m_normals);
        m_colors = eastl::move(copy.m_colors);
        m_indices = eastl::move(copy.m_indices);
        m_topologyType = copy.m_topologyType;

        m_localBounds = copy.m_localBounds;

        m_vertexBuffer = copy.m_vertexBuffer;
        m_normalsBuffer = copy.m_normalsBuffer;
        m_colorBuffer = copy.m_colorBuffer;
        m_indexBuffer = copy.m_indexBuffer;

        copy.m_vertexBuffer = BGFX_INVALID_HANDLE;
        copy.m_normalsBuffer = BGFX_INVALID_HANDLE;
        copy.m_colorBuffer = BGFX_INVALID_HANDLE;
        copy.m_indexBuffer = BGFX_INVALID_HANDLE;

        return *this;
    }

    void Primitive::Destroy()
    {
        if (bgfx::isValid(m_vertexBuffer)) bgfx::destroy(m_vertexBuffer);
        if (bgfx::isValid(m_normalsBuffer)) bgfx::destroy(m_normalsBuffer);
        if (bgfx::isValid(m_colorBuffer)) bgfx::destroy(m_colorBuffer);
        if (bgfx::isValid(m_indexBuffer)) bgfx::destroy(m_indexBuffer);

        m_vertexBuffer = BGFX_INVALID_HANDLE;
        m_normalsBuffer = BGFX_INVALID_HANDLE;
        m_colorBuffer = BGFX_INVALID_HANDLE;
        m_indexBuffer = BGFX_INVALID_HANDLE;
    }

    Primitive::~Primitive()
    {
        Destroy();
    }

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
    }

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

    void Primitive::CreateBuffers()
    {
        if (!m_vertices.empty()) 
        {
	        uint32_t size = uint32_t(sizeof(Vec3f) * m_vertices.size());
            m_vertexBuffer = bgfx::createVertexBuffer(bgfx::makeRef(m_vertices.data(), size), s_vertLayout);
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

    Primitive Primitive::NewPlainTriangle()
    {
        Primitive prim;
        prim.m_vertices = {
            Vec3f(-1.0f, -1.0f, 0.0f),
            Vec3f(1.f, -1.f, 0.0f),
            Vec3f(0.f, 1.f, 0.0f)
        };
        prim.m_colors = {
            Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
            Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
            Vec4f(1.0f, 1.0f, 1.0f, 1.0f)
        };
        prim.m_indices = {0, 1, 2};

        prim.m_topologyType = TopologyType::TriangleList;
        prim.RecalcLocalBounds();
        prim.CreateBuffers();
        return prim;
    }

    Primitive Primitive::NewPlainQuad()
    {
        Primitive prim;
        prim.m_vertices = {
            Vec3f(-1.0f, -1.0f, 0.0f),
            Vec3f(1.f, -1.f, 0.0f),
            Vec3f(-1.f, 1.f, 0.0f),
            Vec3f(1.f, 1.f, 0.0f)
        };
        prim.m_colors = {
            Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
            Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
            Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
            Vec4f(1.0f, 1.0f, 1.0f, 1.0f)
        };
        prim.m_indices = {0, 1, 2, 3};

        prim.m_topologyType = TopologyType::TriangleStrip;
        prim.RecalcLocalBounds();
        prim.CreateBuffers();
        return prim;
    }

    Primitive Primitive::NewCube()
    {
        Primitive prim;
        prim.m_vertices = {
            Vec3f(-1.0f, -1.0f,  1.0f), // Front bottom left
            Vec3f( 1.0f, -1.0f,  1.0f), // Front bottom right
            Vec3f(-1.0f,  1.0f,  1.0f), // Front top left
            Vec3f( 1.0f,  1.0f,  1.0f), // Front top right

            Vec3f(-1.0f, -1.0f, -1.0f), // Back bottom left
            Vec3f( 1.0f, -1.0f, -1.0f), // Back bottom right
            Vec3f(-1.0f,  1.0f, -1.0f), // Back top left
            Vec3f( 1.0f,  1.0f, -1.0f) // Back top right
        };
        prim.m_normals = {
            Vec3f(), Vec3f(), Vec3f(), Vec3f(), Vec3f(), Vec3f(), Vec3f(), Vec3f()
        };
        prim.m_colors = {
            Vec4f(1.0f, 0.0f, 0.0f, 1.0f),
            Vec4f(0.0f, 1.0f, 0.0f, 1.0f),
            Vec4f(1.0f, 0.0f, 1.0f, 1.0f),
            Vec4f(1.0f, 1.0f, 1.0f, 1.0f),

            Vec4f(1.0f, 0.0f, 0.0f, 1.0f),
            Vec4f(0.0f, 1.0f, 0.0f, 1.0f),
            Vec4f(1.0f, 0.0f, 1.0f, 1.0f),
            Vec4f(1.0f, 1.0f, 1.0f, 1.0f)
        };
        prim.m_indices = {
            0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1
        };
        prim.m_topologyType = TopologyType::TriangleStrip;
        prim.RecalcLocalBounds();
        prim.CreateBuffers();
        return prim;
    }
}