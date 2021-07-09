project "Game"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++14"
	exceptionhandling "Off"
	rtti "Off"
    debugdir "../"
	files 
    {
        "Source/**.cpp",
        "Source/**.h"
    }
	includedirs
	{
		"../Engine/Source",
		"../Engine/Lib/SDL2-2.0.8/include",
        "Source/"
	}
    links { "Engine", "bgfx", "bimg", "bx", "SDL2", "SDL2main" }
    filter "platforms:x86_64"
        libdirs { "../Engine/Lib/SDL2-2.0.8/lib/x64" }
    filter "platforms:x86"
        libdirs { "../Engine/Lib/SDL2-2.0.8/lib/x86" }
	filter "system:windows"
		links { "gdi32", "kernel32", "psapi" }
	filter "system:linux"
		links { "dl", "GL", "pthread", "X11" }
	filter "system:macosx"
		links { "QuartzCore.framework", "Metal.framework", "Cocoa.framework", "IOKit.framework", "CoreVideo.framework" }