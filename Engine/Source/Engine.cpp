// Copyright 2020-2021 David Colson. All rights reserved.

#include "Engine.h"

#include "Log.h"
#include "Memory.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <EASTL/vector.h>


void MakeWindow()
{
	eastl::vector<float> vec;
	
	An::Log::Debug("Hello");

    SDL_Window* pWindow = SDL_CreateWindow(
		"Flight Game",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		int(800),
		int(500),
		SDL_WINDOW_RESIZABLE
	);

	bgfx::renderFrame();

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(pWindow, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	bgfx::Init init;
	init.platformData.ndt = NULL;
	init.platformData.nwh = wmInfo.info.win.window;

	bgfx::init(init);
	const bgfx::ViewId kClearView = 0;
	bgfx::setViewClear(kClearView, BGFX_CLEAR_COLOR);
	bgfx::setViewRect(kClearView, 0, 0, bgfx::BackbufferRatio::Equal);
	bgfx::reset(800, 500, BGFX_RESET_VSYNC);
    bool gameRunning = true;
    while (gameRunning)
	{
		bgfx::touch(kClearView);

		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(10, 10, 0x0f, "Hello world");
		//bgfx::setDebug(BGFX_DEBUG_TEXT);
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