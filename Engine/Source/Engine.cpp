// Copyright 2020-2021 David Colson. All rights reserved.

#include "Engine.h"

#include "Core/Log.h"
#include "Core/Memory.h"
#include "AssetDatabase/AssetDatabase.h"
#include "AssetDatabase/Text.h"
#include "AssetDatabase/Shader.h"
#include "AssetDatabase/Mesh.h"
#include "Core/Vec3.h"
#include "Core/Matrix.h"

#include <SDL.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <EASTL/vector.h>

#include <shaderc/shaderc.h>

// This defines a macro called min somehow? We should avoid it at all costs and include it last
#include <SDL_syswm.h>

struct PosColVert
{
	Vec3f m_pos;
	uint32_t m_abgr;

	static void init()
	{
		posLayout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.end();

		colLayout
			.begin()
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
	};

	static bgfx::VertexLayout posLayout;
	static bgfx::VertexLayout colLayout;
};
bgfx::VertexLayout PosColVert::posLayout;
bgfx::VertexLayout PosColVert::colLayout;

void MakeWindow()
{
	eastl::vector<float> vec;
	
	int winWidth = 1600;
	int winHeight = 900;

	An::Log::Debug("Hello");

    SDL_Window* pWindow = SDL_CreateWindow(
		"Flight Game",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		int(winWidth),
		int(winHeight),
		SDL_WINDOW_RESIZABLE
	);

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(pWindow, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	bgfx::Init init;
	init.type = bgfx::RendererType::Direct3D11;
	init.platformData.ndt = NULL;
	init.platformData.nwh = wmInfo.info.win.window;

	bgfx::init(init);
	const bgfx::ViewId kClearView = 0;
	bgfx::setViewClear(kClearView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x404040ff, 1.0f, 0);

	An::AssetHandle cubesVertShader = An::AssetHandle("Engine/Shaders/cubes.vs");
	An::AssetHandle cubesFragShader = An::AssetHandle("Engine/Shaders/cubes.fs");
	
	// Create cube input layout
	An::Primitive::InitPrimitiveLayouts();
	An::Primitive cube = An::Primitive::NewCube();

	bgfx::ProgramHandle program = bgfx::createProgram(An::AssetDB::GetAsset<An::Shader>(cubesVertShader)->m_handle, An::AssetDB::GetAsset<An::Shader>(cubesFragShader)->m_handle, false);

	bgfx::setViewRect(kClearView, 0, 0, winWidth, winHeight);
	bgfx::reset(winWidth, winHeight, BGFX_RESET_VSYNC);
    bool gameRunning = true;
    while (gameRunning)
	{
		bgfx::touch(kClearView);
		
		// TODO: Play around with vertex buffer streams so we can separate colour and position in the input layout (note: see mvs demo)
		Matrixf camera = Matrixf::MakeTranslation(Vec3f(0.0f, 0.0f, 5.0f));
		Matrixf project = Matrixf::Perspective((float)winWidth, (float)winHeight, 0.1f, 100.0f, 60.0f);

		bgfx::setViewTransform(0, &camera, &project);


		uint64_t state = 0
				| BGFX_STATE_WRITE_R
				| BGFX_STATE_WRITE_G
				| BGFX_STATE_WRITE_B
				| BGFX_STATE_WRITE_A
				| BGFX_STATE_WRITE_Z
				| BGFX_STATE_DEPTH_TEST_LESS
				| BGFX_STATE_CULL_CCW
				| BGFX_STATE_MSAA
				| BGFX_STATE_PT_TRISTRIP;

		Matrixf rotate = Matrixf::MakeRotation(Vec3f(-0.25f, -0.37f, 0.0f));

		// Set model matrix for rendering.
		bgfx::setTransform(&rotate);

		bgfx::setVertexBuffer(0, cube.m_vertexBuffer);
		bgfx::setVertexBuffer(1, cube.m_colorBuffer);
		bgfx::setIndexBuffer(cube.m_indexBuffer);
		bgfx::setState(state);
		bgfx::submit(0, program);

		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(10, 10, 0x0f, "Hello world");
		bgfx::setDebug(BGFX_DEBUG_TEXT);
		bgfx::frame();

		// Deal with events
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_CLOSE:
					gameRunning = false;
					break;
				default:
					break;
				}
				break;
			case SDL_QUIT:
				gameRunning = false;
				break;
			}
		}
    }
	cube.Destroy();
	bgfx::shutdown();
}