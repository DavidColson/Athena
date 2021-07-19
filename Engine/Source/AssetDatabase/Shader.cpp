#include "Shader.h"

#include "Core/Log.h"

#include <shaderc/shaderc.h>

namespace An
{
	// ***********************************************************************

    Shader::Shader()
    {
        bOverrideReload = true;
    }

	// ***********************************************************************

    void Shader::Load(Path path, AssetHandle handleForThis)
    {
        const eastl::string extension = path.Extension().AsString();

        const bgfx::Memory* pShaderMem = nullptr;
        if (extension == ".fs")
        {   
            m_type = Fragment;
            pShaderMem = shaderc::compileShader(shaderc::ST_FRAGMENT, path.AsRawString(), "", "Engine/Shaders/varying.def.sc");
        }
        else if (extension == ".vs")
        {
            m_type = Vertex;
            pShaderMem = shaderc::compileShader(shaderc::ST_VERTEX, path.AsRawString(), "", "Engine/Shaders/varying.def.sc");
        }

        if (pShaderMem != nullptr)
        {
            m_handle = bgfx::createShader(pShaderMem);
        }
        else
        {
            Log::Crit("Failed to compile shader %s", path.AsRawString());
        }
    }

	// ***********************************************************************

    void Shader::Reload(Path path, AssetHandle handleForThis)
    {
        const bgfx::Memory* pNewShaderMem = nullptr;
        switch (m_type)
        {
        case Fragment:
            pNewShaderMem = shaderc::compileShader(shaderc::ST_FRAGMENT, path.AsRawString(), "", "Engine/Shaders/varying.def.sc");
            break;
        case Vertex:
            pNewShaderMem = shaderc::compileShader(shaderc::ST_VERTEX, path.AsRawString(), "", "Engine/Shaders/varying.def.sc");
            break;
        default:
            break;
        }

        if (pNewShaderMem != nullptr)
        {
            bgfx::destroy(m_handle);
            m_handle = bgfx::createShader(pNewShaderMem);
        }
    }

	// ***********************************************************************

    Shader::~Shader()
    {
        bgfx::destroy(m_handle);
    }
}