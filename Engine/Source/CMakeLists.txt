project(Engine)


include_directories("ThirdParty/")
include_directories("ThirdParty/EASTL/include/")
include_directories("ThirdParty/EABase/include/Common/")
include_directories("ThirdParty/bgfx/bgfx/include/")
include_directories("ThirdParty/bgfx/bx/include/")
include_directories("ThirdParty/bgfx/bimg/include/")
add_Library(Engine "")

target_sources(Engine
    PRIVATE
        "Engine.h"
        "Engine.cpp"
)

add_subdirectory(ThirdParty/EASTL)
add_subdirectory(ThirdParty/EABase)
add_subdirectory(ThirdParty/bgfx)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

target_precompile_headers(Engine PRIVATE "PreCompiledHeader.h")

target_link_libraries(Engine ${SDL2_LIBRARIES} EASTL bgfx)

if(MSVC)
  target_compile_options(Engine PRIVATE /W3 /WX)
else()
  target_compile_options(Engine PRIVATE -Wall -Werror)
endif()

# target_compile_options(Engine PRIVATE /Bt)
# target_compile_options(Engine PRIVATE /showIncludes)