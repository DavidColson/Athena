#pragma once

#include "Core/Vec3.h"
#include "Core/Vec4.h"
#include "Core/Vec2.h"
#include "Core/Quat.h"
#include "Core/AABB.h"
#include "Core/Path.h"
#include "Mesh.h"

#include <bgfx/bgfx.h>
#include <EASTL/vector.h>

namespace An
{
    struct Node
    {
        eastl::string m_name;
        
        Node* m_pParent{ nullptr };
        eastl::vector<Node*> m_children;

        uint32_t m_meshId;

        Vec3f m_translation;
        Vec3f m_scale;
        Quatf m_rotation;
        
        Matrixf m_worldTransform;
    };

    struct Scene
    {
        Scene() {}
        Scene(Path path);

        Quatf m_cameraRotation;
        Vec3f m_cameraTranslation;

        eastl::vector<Mesh> m_meshes;
        eastl::vector<Node> m_nodes;
    };
}