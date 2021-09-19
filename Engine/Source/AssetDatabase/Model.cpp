#include "Model.h"

#include "Core/Json.h"
#include "Core/Base64.h"

#include <SDL_rwops.h>

namespace An
{
    // Actually owns the data
    struct Buffer
    {
        char* pBytes{ nullptr };
        size_t byteLength{ 0 };
    };

    // Does not actually own the data
    struct BufferView
    {
        // pointer to some place in a buffer
        char* pBuffer{ nullptr };
        size_t length{ 0 };

        enum Target
        {
            Array,
            ElementArray
        };
        Target target;    
    };

    struct Accessor
    {
        // pointer to some place in a buffer view
        char* pBuffer{ nullptr };
        int count{ 0 };
        enum ComponentType
        {
            Byte,
            UByte,
            Short,
            UShort,
            UInt,
            Float
        };
        ComponentType componentType;

        enum Type
        {
            Scalar,
            Vec2,
            Vec3,
            Vec4,
            Mat2,
            Mat3,
            Mat4
        };
        Type type;
    };

    void ParseNodesRecursively(Node* pParent, eastl::vector<Node>& outNodes, JsonValue& nodeToParse, JsonValue& nodesData)
    {
        for (int i = 0; i < nodeToParse.Count(); i++)
        {
            int nodeId = nodeToParse[i].ToInt();
            JsonValue& jsonNode = nodesData[nodeId];

            // extract the nodes
            outNodes.emplace_back();
            Node& node = outNodes.back();

            node.m_name = jsonNode.HasKey("name") ? jsonNode["name"].ToString() : "";
            node.m_meshId = UINT32_MAX;

            if (jsonNode.HasKey("children"))
            {
                ParseNodesRecursively(&node, outNodes, jsonNode["children"], nodesData);
            }

            if (pParent)
            {
                pParent->m_children.push_back(&outNodes.back());
                outNodes.back().m_pParent = pParent;
            }

            if (jsonNode.HasKey("mesh"))
            {
                node.m_meshId = jsonNode["mesh"].ToInt();
            }

            if (jsonNode.HasKey("rotation"))
            {
                node.m_rotation.x = float(jsonNode["rotation"][0].ToFloat());
                node.m_rotation.y = float(jsonNode["rotation"][1].ToFloat());
                node.m_rotation.z = float(jsonNode["rotation"][2].ToFloat());
                node.m_rotation.w = float(jsonNode["rotation"][3].ToFloat());
            }
            else
            {
                node.m_rotation = Quatf::Identity();
            }

            if (jsonNode.HasKey("translation"))
            {
                node.m_translation.x = float(jsonNode["translation"][0].ToFloat());
                node.m_translation.y = float(jsonNode["translation"][1].ToFloat());
                node.m_translation.z = float(jsonNode["translation"][2].ToFloat());
            }
            else
            {
                node.m_translation = Vec3f(0.0f);
            }

            if (jsonNode.HasKey("scale"))
            {
                node.m_scale.x = float(jsonNode["scale"][0].ToFloat());
                node.m_scale.y = float(jsonNode["scale"][1].ToFloat());
                node.m_scale.z = float(jsonNode["scale"][2].ToFloat());
            }
            else
            {
                node.m_scale = Vec3f(1.0f);
            }
        } 
    }

    Scene::Scene(Path path)
    {
        SDL_RWops* pFileRead = SDL_RWFromFile(path.AsRawString(), "rb");

        uint64_t size = SDL_RWsize(pFileRead);
        char* pData = new char[size];
        SDL_RWread(pFileRead, pData, size, 1);
        SDL_RWclose(pFileRead);

        eastl::string file(pData, pData + size);

        JsonValue parsed = ParseJsonFile(file);

        bool validGltf = parsed["asset"]["version"].ToString() == "2.0";
        if (!validGltf)
            return;

        eastl::vector<Buffer> rawDataBuffers;
        JsonValue& jsonBuffers = parsed["buffers"];
        for (int i = 0; i < jsonBuffers.Count(); i++)
        {
            Buffer buf;
            buf.byteLength = jsonBuffers[i]["byteLength"].ToInt();
            buf.pBytes = new char[buf.byteLength];
            
            eastl::string encodedBuffer = jsonBuffers[i]["uri"].ToString().substr(37);
            memcpy(buf.pBytes, DecodeBase64(encodedBuffer).data(), buf.byteLength);

            rawDataBuffers.push_back(buf);
        }

        eastl::vector<BufferView> bufferViews;
        JsonValue& jsonBufferViews = parsed["bufferViews"];

        for (int i = 0; i < jsonBufferViews.Count(); i++)
        {
            BufferView view;

            int bufIndex = jsonBufferViews[i]["buffer"].ToInt();
            view.pBuffer = rawDataBuffers[bufIndex].pBytes + jsonBufferViews[i]["byteOffset"].ToInt(); //@Incomplete, byte offset could not be provided, in which case we assume 0

            view.length = jsonBufferViews[i]["byteLength"].ToInt();

            // @Incomplete, target may not be provided
            int target = jsonBufferViews[i]["target"].ToInt();
            if (target == 34963)
                view.target = BufferView::ElementArray;
            else if (target = 34962)
                view.target = BufferView::Array;
            bufferViews.push_back(view);
        }

        eastl::vector<Accessor> accessors;
        JsonValue& jsonAccessors = parsed["accessors"];
        accessors.reserve(jsonAccessors.Count());

        for (int i = 0; i < jsonAccessors.Count(); i++)
        {
            Accessor acc;
            JsonValue& jsonAcc = jsonAccessors[i];

            int idx = jsonAcc["bufferView"].ToInt();
            acc.pBuffer = bufferViews[idx].pBuffer + jsonAcc["byteOffset"].ToInt();
            
            acc.count = jsonAcc["count"].ToInt();

            int compType = jsonAcc["componentType"].ToInt();
            switch (compType)
            {
            case 5120: acc.componentType = Accessor::Byte; break;
            case 5121: acc.componentType = Accessor::UByte; break;
            case 5122: acc.componentType = Accessor::Short; break;
            case 5123: acc.componentType = Accessor::UShort; break;
            case 5125: acc.componentType = Accessor::UInt; break;
            case 5126: acc.componentType = Accessor::Float; break;
            default: break;
            }

            eastl::string type = jsonAcc["type"].ToString();
            if (type == "SCALAR") acc.type = Accessor::Scalar;
            else if (type == "VEC2") acc.type = Accessor::Vec2;
            else if (type == "VEC3") acc.type = Accessor::Vec3;
            else if (type == "VEC4") acc.type = Accessor::Vec4;
            else if (type == "MAT2") acc.type = Accessor::Mat2;
            else if (type == "MAT3") acc.type = Accessor::Mat3;
            else if (type == "MAT4") acc.type = Accessor::Mat4;

            accessors.push_back(acc);
        }
        
        m_nodes.reserve(parsed["nodes"].Count());
        ParseNodesRecursively(nullptr, m_nodes, parsed["scenes"][0]["nodes"], parsed["nodes"]);

        m_meshes.reserve(parsed["meshes"].Count());
        for (int i = 0; i < parsed["meshes"].Count(); i++)
        {
            JsonValue& jsonMesh = parsed["meshes"][i];

            Mesh mesh;
            mesh.m_name = jsonMesh.HasKey("name") ? jsonMesh["name"].ToString() : "";

            for (int j = 0; j < jsonMesh["primitives"].Count(); j++)
            {
                JsonValue& jsonPrimitive = jsonMesh["primitives"][j];
                Primitive prim;

                if (jsonPrimitive.HasKey("mode"))
                {
                    if (jsonPrimitive["mode"].ToInt() != 4)
                    {
                        return; // Unsupported topology type
                    }
                }

                int nVerts = accessors[jsonPrimitive["attributes"]["POSITION"].ToInt()].count;

                JsonValue& jsonAttr = jsonPrimitive["attributes"];
                Vec3f* vertPositionBuffer = (Vec3f*)accessors[jsonAttr["POSITION"].ToInt()].pBuffer;
                prim.m_vertices = eastl::vector<Vec3f>(vertPositionBuffer, vertPositionBuffer + nVerts);

                Vec3f* vertNormBuffer = jsonAttr.HasKey("NORMAL") ? (Vec3f*)accessors[jsonAttr["NORMAL"].ToInt()].pBuffer : nullptr;
                prim.m_normals = eastl::vector<Vec3f>(vertNormBuffer, vertNormBuffer + nVerts);

                Vec2f* vertTexCoordBuffer = jsonAttr.HasKey("TEXCOORD_0") ? (Vec2f*)accessors[jsonAttr["TEXCOORD_0"].ToInt()].pBuffer : nullptr;
                prim.m_uv0 = eastl::vector<Vec2f>(vertTexCoordBuffer, vertTexCoordBuffer + nVerts);
                
                if (jsonAttr.HasKey("COLOR_0"))
                {
                    Vec4f* vertColBuffer = (Vec4f*)accessors[jsonAttr["COLOR_0"].ToInt()].pBuffer;
                    prim.m_colors = eastl::vector<Vec4f>(vertColBuffer, vertColBuffer + nVerts);
                }

                int nIndices = accessors[jsonPrimitive["indices"].ToInt()].count;
                uint16_t* indexBuffer = (uint16_t*)accessors[jsonPrimitive["indices"].ToInt()].pBuffer;
                prim.m_indices = eastl::vector<uint16_t>(indexBuffer, indexBuffer + nIndices);

                prim.RecalcLocalBounds();
                prim.CreateBuffers();
                mesh.m_primitives.push_back(eastl::move(prim));
            }
            m_meshes.push_back(eastl::move(mesh));
        }
        
        for (int i = 0; i < rawDataBuffers.size(); i++)
        {
            delete rawDataBuffers[i].pBytes;
        }
    }
}