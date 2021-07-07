#include "Engine.h"

#include <SDL.h>

void MakeWindow()
{
    SDL_Window* pWindow = SDL_CreateWindow(
		"Flight Game",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		int(800),
		int(500),
		SDL_WINDOW_RESIZABLE
	);

    bool gameRunning = true;

    while (gameRunning)
	{
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
}