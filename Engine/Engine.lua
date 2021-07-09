
-- Note to dave, paths are relative to the location of THIS file

project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++14"
    exceptionhandling "Off"
    rtti "Off"
    files 
    {
        "Source/*.cpp",
        "Source/*.h"
    }
    includedirs
    {
        "Source/",
        "Source/ThirdParty/bgfx/include",
        "Source/ThirdParty/bimg/include",
        "Source/ThirdParty/bx/include",
        "Lib/SDL2-2.0.8/include",
    }
    defines 
    {
        "WINDOWS_IGNORE_PACKING_MISMATCH"
    }