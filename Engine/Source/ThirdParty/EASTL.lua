
group "ThirdParty/EASTL"

project "EASTL"
    kind "StaticLib"
    language "C++"
    cppdialect "C++14"
    exceptionhandling "Off"
    rtti "Off"
    files
    {
        "EASTL/source/*.cpp",
        "EASTL/include/**.h"
    }
    includedirs
    {
        "EASTL/include",
        "EABase/include/Common"
    }
    links 
    {
        EABase
    }
    defines
    {
        "_CHAR16T",
        "_CRT_SECURE_NO_WARNINGS",
        "_SCL_SECURE_NO_WARNINGS",
        "EASTL_OPENSOURCE=1"
    }

group ""