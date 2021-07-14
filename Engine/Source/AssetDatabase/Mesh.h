#pragma once

#include "Core/Vec3.h"
#include "Core/Vec4.h"
#include "Core/AABB.h"
#include "AssetDatabase.h"

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

        // Primitive Mesh Creation
        // TODO: These should really return Mesh's not primitives.
        static Primitive NewPlainQuad();
        static Primitive NewPlainTriangle();
        static Primitive NewCube();

        eastl::string m_name{"Primitive"};

        eastl::vector<Vec3f> m_vertices;
        eastl::vector<Vec3f> m_normals;
        eastl::vector<Vec4f> m_colors;
        eastl::vector<uint16_t> m_indices{ nullptr };

        TopologyType m_topologyType{TopologyType::TriangleList};
        bgfx::VertexBufferHandle m_vertexBuffer;
        bgfx::VertexBufferHandle m_normalsBuffer;
        bgfx::VertexBufferHandle m_colorBuffer;
        bgfx::IndexBufferHandle m_indexBuffer;

        AABBf m_localBounds;

        static bgfx::VertexLayout s_vertLayout;
	    static bgfx::VertexLayout s_normLayout;
	    static bgfx::VertexLayout s_colLayout;
    };

    struct Mesh : public Asset
    {
        virtual void Load(Path path, AssetHandle handleForThis) override;

        eastl::string m_name;
        eastl::vector<Primitive> m_primitives;

        void CreateBuffers();
    };
}