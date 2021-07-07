cmake_minimum_required(VERSION 3.13)

project (Athena)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Build/")

set(ENGINE_SOURCE_PATH "${CMAKE_SOURCE_DIR}/Engine/Source/")

set(SDL2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/Engine/Lib/SDL2-2.0.8/include/")
set(SDL2_LIB_DIRS "${CMAKE_SOURCE_DIR}/Engine/Lib/SDL2-2.0.8/lib/x64/")
set(SDL2_LIBRARIES SDL2 SDL2main)

macro(GroupSources dir)
    file(GLOB_RECURSE sources RELATIVE ${dir} *.h *.hpp *.c *.cpp *.cc)
    foreach(source ${sources})
        get_filename_component(source_path "${source}" PATH)
        string(REPLACE "/" "\\" source_path_msvc "${source_path}")
        source_group("${source_path_msvc}" FILES "${source}")
    endforeach()
endmacro()


# Find d3d headers and libs and set variables
link_directories(${SDL2_LIB_DIRS})
include_directories(${SDL2_INCLUDE_DIRS})

add_subdirectory(Engine/Source)
add_subdirectory(Game/Source)