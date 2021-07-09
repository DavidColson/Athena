
local BUILD_DIR = path.join("Build", _ACTION)
if _OPTIONS["cc"] ~= nil then
	BUILD_DIR = BUILD_DIR .. "_" .. _OPTIONS["cc"]
end

GAME_DIR    = "Game/Source"
ENGINE_DIR  = "Engine/Source"

-- Third party locations
SDL_DIR     = "Engine/Lib/SDL2-2.0.8"
BGFX_DIR    = "Engine/Source/ThirdParty/bgfx"
BIMG_DIR    = "Engine/Source/ThirdParty/bimg"
BX_DIR      = "Engine/Source/ThirdParty/bx"
EABASE_DIR  = "Engine/Source/ThirdParty/EABase"
EASTL_DIR   = "Engine/Source/ThirdParty/EASTL"

function setBxCompat()
	filter "action:vs*"
		includedirs { path.join(BX_DIR, "include/compat/msvc") }
	filter { "system:windows", "action:gmake" }
		includedirs { path.join(BX_DIR, "include/compat/mingw") }
	filter { "system:macosx" }
		includedirs { path.join(BX_DIR, "include/compat/osx") }
		buildoptions { "-x objective-c++" }
end

solution "Athena"
    location(BUILD_DIR)
    startproject "Game"
    configurations { "Release", "Debug" }
    if os.is64bit() and not os.istarget("windows") then
        platforms "x86_64"
    else
        platforms { "x86", "x86_64" }
    end
    filter "configurations:Release"
        defines "NDEBUG"
        optimize "Full"
    filter "configurations:Debug*"
        defines "_DEBUG"
        optimize "Debug"
        symbols "On"
    filter "platforms:x86"
        architecture "x86"
    filter "platforms:x86_64"
        architecture "x86_64"
    filter "system:macosx"
        xcodebuildsettings {
            ["MACOSX_DEPLOYMENT_TARGET"] = "10.9",
            ["ALWAYS_SEARCH_USER_PATHS"] = "YES", -- This is the minimum version of macos we'll be able to run on
        };
    
    dofile("Engine/Source/ThirdParty/bgfx.lua")
    dofile("Engine/Source/ThirdParty/bx.lua")
    dofile("Engine/Source/ThirdParty/bimg.lua")
    dofile("Engine/Engine.lua")
    dofile("Game/Game.lua")