project(Game)

include_directories("${ENGINE_SOURCE_PATH}/")

add_executable (Game WIN32 "")
target_link_libraries(Game Engine)

target_sources(Game
    PRIVATE
        "Game.h"
        "Game.cpp"
)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

target_precompile_headers(Engine PRIVATE "${ENGINE_SOURCE_PATH}/PreCompiledHeader.h")

if(MSVC)
  target_compile_options(Game PRIVATE /W3 /WX)
else()
  target_compile_options(Game PRIVATE -Wall -Werror)
endif()

# target_link_options(Game PRIVATE /time+)
# target_compile_options(Game PRIVATE /Bt+ /d2cgsummary)