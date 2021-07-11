// Copyright 2020-2021 David Colson. All rights reserved.

#include "Engine.h"

#include "Core/Log.h"
#include "Core/Memory.h"
#include "AssetDatabase/AssetDatabase.h"
#include "AssetDatabase/Text.h"
#include "Core/Vec3.h"
#include "Core/Matrix.h"

#include <SDL.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <EASTL/vector.h>

#include <shaderc/shaderc.h>
#include <bx/bx.h>

// This defines a macro called min somehow? We should avoid it at all costs and include it last
#include <SDL_syswm.h>

struct PosColVert
{
	Vec3f m_pos;
	uint32_t m_abgr;

	static void init()
	{
		ms_layout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
			.end();
	};

	static bgfx::VertexLayout ms_layout;
};
bgfx::VertexLayout PosColVert::ms_layout;

void MakeWindow()
{
	eastl::vector<float> vec;
	
	int winWidth = 1200;
	int winHeight = 800;

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

	const bgfx::Memory* memVs = shaderc::compileShader(shaderc::ST_VERTEX, "Engine/Shaders/cubes.vs", "", "Engine/Shaders/varying.def.sc");
	bgfx::ShaderHandle vsh = bgfx::createShader(memVs);

	const bgfx::Memory* memFs = shaderc::compileShader(shaderc::ST_FRAGMENT, "Engine/Shaders/cubes.fs", "", "Engine/Shaders/varying.def.sc");
	bgfx::ShaderHandle fsh = bgfx::createShader(memFs);

	// Create cube input layout
	PosColVert::init();
	
	eastl::vector<PosColVert> verts = {
		{ Vec3f(-1.0f, -1.0f,  1.0f), 0xff000000 }, // Front bottom left
        { Vec3f( 1.0f, -1.0f,  1.0f), 0xff0000ff }, // Front bottom right
        { Vec3f(-1.0f,  1.0f,  1.0f), 0xff00ff00 }, // Front top left
        { Vec3f( 1.0f,  1.0f,  1.0f), 0xff00ffff }, // Front 

       	{ Vec3f(-1.0f, -1.0f, -1.0f), 0xffff0000 }, // Back bottom left
        { Vec3f( 1.0f, -1.0f, -1.0f), 0xffff00ff }, // Back bottom right
        { Vec3f(-1.0f,  1.0f, -1.0f), 0xffffff00 }, // Back top left
        { Vec3f( 1.0f,  1.0f, -1.0f), 0xffffff00 }  // Back top right
    };

	eastl::vector<uint16_t> indices = {
		0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1
	};

	uint32_t vbsize = uint32_t(sizeof(PosColVert) * verts.size());
	bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(bgfx::makeRef(verts.data(), vbsize), PosColVert::ms_layout);
	
	uint32_t ibsize = uint32_t(sizeof(uint16_t) * indices.size());
	bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(bgfx::makeRef(indices.data(), ibsize));

	bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh, true);

	bgfx::setViewRect(kClearView, 0, 0, winWidth, winHeight);
	bgfx::reset(winWidth, winHeight, BGFX_RESET_VSYNC);
    bool gameRunning = true;
    while (gameRunning)
	{
		bgfx::touch(kClearView);
		
		// TODO: Play around with vertex buffer streams so we can separate colour and position in the input layout
		Matrixf camera = Matrixf::MakeTranslation(Vec3f(0.0f, 0.0f, 20.0f));
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

		float mtx[16];
		bx::mtxRotateXY(mtx, 0.21f, 0.37f);

		// Set model matrix for rendering.
		bgfx::setTransform(mtx);


		bgfx::setVertexBuffer(0, vbh);
		bgfx::setIndexBuffer(ibh);
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
	bgfx::shutdown();
}