project "fcpp"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
	exceptionhandling "Off"
	rtti "Off"
	files
	{
		"bgfx/3rdparty/fcpp/*.h",
		"bgfx/3rdparty/fcpp/*.cpp",
	}
	includedirs
	{
		"bgfx/3rdparty/fcpp/"
	}
	defines
	{
		"NINCLUDE=64",
		"NWORK=65536",
		"NBUFF=65536",
		"OLD_PREPROCESSOR=0"
	}
	setBxCompat()

project "bgfx"
    kind "StaticLib"
    language "C++"
    cppdialect "C++14"
    exceptionhandling "Off"
    rtti "Off"
    defines "__STDC_FORMAT_MACROS"
    files
    {
        "bgfx/include/bgfx/**.h",
        "bgfx/src/*.cpp",
        "bgfx/src/*.h",
    }
    excludes
    {
        "bgfx/src/amalgamated.cpp",
    }
    includedirs
    {
        "bx/include",
        "bimg/include",
        "bgfx/include",
        "bgfx/3rdparty",
        "bgfx/3rdparty/dxsdk/include",
        "bgfx/3rdparty/khronos"
    }
    filter "configurations:Debug"
        defines "BGFX_CONFIG_DEBUG=1"
    filter "action:vs*"
        defines "_CRT_SECURE_NO_WARNINGS"
        excludes
        {
            "bgfx/src/glcontext_glx.cpp",
            "bgfx/src/glcontext_egl.cpp"
        }
    filter "system:macosx"
        files
        {
            "bgfx/src/*.mm",
        }
    filter "action:vs*"
		includedirs { "bx/include/compat/msvc" }
	filter { "system:windows", "action:gmake" }
		includedirs { "bx/include/compat/mingw" }
	filter { "system:macosx" }
		includedirs { "bx/include/compat/osx" }
		buildoptions { "-x objective-c++" }