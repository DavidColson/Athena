
-- Note to dave, paths are relative to the location of THIS file

project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    exceptionhandling "Off"
    rtti "Off"
    files 
    {
        "Source/*.cpp",
        "Source/*.h",
        "Source/Core/**.cpp",
        "Source/Core/**.h",
        "Source/AssetDatabase/**.cpp",
        "Source/AssetDatabase/**.h",
    }
    includedirs
    {
        "Source/",
        "Source/ThirdParty/bgfx/include",
        "Source/ThirdParty/bimg/include",
        "Source/ThirdParty/bx/include",
        "Source/ThirdParty/bgfx/tools",
        "Source/ThirdParty/EABase/include/Common",
        "Source/ThirdParty/EASTL/include",
        "Lib/SDL2-2.0.8/include",
    }
    defines 
    {
        "WINDOWS_IGNORE_PACKING_MISMATCH",
		"__STDC_LIMIT_MACROS",
		"__STDC_FORMAT_MACROS",
		"__STDC_CONSTANT_MACROS",
	}
    filter "action:vs*"
        defines "_CRT_SECURE_NO_WARNINGS"
        includedirs { "Source/ThirdParty/bx/include/compat/msvc" }
    filter { "system:windows", "action:gmake" }
        includedirs { "Source/ThirdParty/bx/include/compat/mingw" }
    filter { "system:macosx" }
        includedirs { "Source/ThirdParty/bx/include/compat/osx" }
        buildoptions { "-x objective-c++" }