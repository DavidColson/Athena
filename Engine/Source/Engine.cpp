// Copyright 2020-2021 David Colson. All rights reserved.

#include "Engine.h"

#include "Core/Log.h"
#include "Core/Memory.h"
#include "TypeSystem/TypeDatabase.h"
#include "Core/Vec2.h"
#include "Input.h"
#include "AssetDatabase/Mesh.h"

#include <SDL.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <EASTL/vector.h>

#include <shaderc/shaderc.h>

// This defines a macro called min somehow? We should avoid it at all costs and include it last
#include <SDL_syswm.h>

namespace An
{
	namespace 
	{
		bool gameRunning{ true };
		uint64_t frameStartTime;
		float deltaTime;
		Vec2i relativeMouseStartLocation{ Vec2i(0, 0) };
		bool isCapturingMouse{ false };
		const bgfx::ViewId kClearView{ 0 };
	}

	void InitWindow(int width, int height)
	{
		SDL_Window* pWindow = SDL_CreateWindow(
			"Flight Game",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width,
			height,
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

		bgfx::renderFrame();

		bgfx::init(init);
		bgfx::setViewClear(kClearView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x404040ff, 1.0f, 0);
		bgfx::setViewRect(kClearView, 0, 0, width, height);
		bgfx::reset(width, height, BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X8);
		An::Primitive::InitPrimitiveLayouts();
		gameRunning = true;
		deltaTime = 0.016f;
	}

    bool ShouldWindowClose()
	{
		return !gameRunning;
	}

	float StartFrame()
	{
		frameStartTime = SDL_GetPerformanceCounter();
		Input::ClearState();

		// Deal with events
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			Input::ProcessEvent(&event);
			switch (event.type)
			{
			case SDL_KEYDOWN:
				{
					if (event.key.keysym.scancode == SDL_SCANCODE_TAB && event.key.keysym.mod & KMOD_LSHIFT)
					{
						isCapturingMouse = !isCapturingMouse;
						if (isCapturingMouse)
						{
							SDL_GetGlobalMouseState(&relativeMouseStartLocation.x, &relativeMouseStartLocation.y);
							SDL_SetRelativeMouseMode(SDL_TRUE);
						}
						else
						{
							SDL_SetRelativeMouseMode(SDL_FALSE);
							SDL_WarpMouseGlobal(relativeMouseStartLocation.x, relativeMouseStartLocation.y);
						}
					}
				}
				break;
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

		bgfx::touch(kClearView);
		return deltaTime;
	}

    void EndFrame()
	{
		bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS);
		bgfx::frame();

		deltaTime = float(SDL_GetPerformanceCounter() - frameStartTime) / SDL_GetPerformanceFrequency();
	}

	void CloseWindow()
	{
		bgfx::shutdown();
	}
}