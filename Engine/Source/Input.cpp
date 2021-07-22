
#include "Input.h"

#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_scancode.h>

struct InputState
{
#define NKEYS 512
	eastl::bitset<NKEYS> keyDowns;
	eastl::bitset<NKEYS> keyUps;
	eastl::bitset<NKEYS> keyStates;

	eastl::bitset<5> mouseDowns;
	eastl::bitset<5> mouseUps;
	eastl::bitset<5> mouseStates;

	float mouseXPos{ 0.0f };
	float mouseYPos{ 0.0f };
	float mouseXDelta{ 0.0f };
	float mouseYDelta{ 0.0f };
};
InputState *pInput;

// ***********************************************************************

void Input::CreateInputState()
{
	pInput = new InputState();
}

// ***********************************************************************

bool Input::GetKeyDown(int keyCode)
{
	return pInput->keyDowns[keyCode];
}

// ***********************************************************************

bool Input::GetKeyUp(int keyCode)
{
	return pInput->keyUps[keyCode];
}

// ***********************************************************************

bool Input::GetKeyHeld(int keyCode)
{
	return pInput->keyStates[keyCode];
}

// ***********************************************************************

bool Input::GetMouseDown(int buttonCode)
{
	return pInput->mouseDowns[buttonCode-1];
}

// ***********************************************************************

bool Input::GetMouseUp(int buttonCode)
{
	return pInput->mouseUps[buttonCode-1];
}

// ***********************************************************************

bool Input::GetMouseHeld(int buttonCode)
{
	return pInput->mouseStates[buttonCode];
}

// ***********************************************************************

Vec2f Input::GetMouseDelta()
{
	return Vec2f(pInput->mouseXDelta, pInput->mouseYDelta);
}

// ***********************************************************************

bool Input::GetMouseInRelativeMode()
{
	if (SDL_GetRelativeMouseMode() == SDL_TRUE)
		return true;
	else
		return false;
}

void Input::ClearState()
{	
	pInput->keyDowns.reset();
	pInput->keyUps.reset();
	pInput->mouseDowns.reset();
	pInput->mouseUps.reset();
	pInput->mouseXDelta = 0.0f;
	pInput->mouseYDelta = 0.0f;
}

// ***********************************************************************

void Input::ClearHeldState()
{
	pInput->keyStates.reset();
	pInput->mouseStates.reset();
}

// ***********************************************************************

void Input::ProcessEvent(SDL_Event* event)
{	
	switch (event->type)
	{
	case SDL_KEYDOWN:
		pInput->keyDowns[event->key.keysym.scancode] = true;
		pInput->keyStates[event->key.keysym.scancode] = true;
		break;
	case SDL_KEYUP:
		pInput->keyUps[event->key.keysym.scancode] = true;
		pInput->keyStates[event->key.keysym.scancode] = false;
		break;
	case SDL_MOUSEMOTION:
		pInput->mouseXPos = (float)event->motion.x;
		pInput->mouseYPos = (float)event->motion.y;
		pInput->mouseXDelta = (float)event->motion.xrel;
		pInput->mouseYDelta = (float)event->motion.yrel;
		break;
	case SDL_MOUSEBUTTONDOWN:
		pInput->mouseDowns[event->button.button] = true;
		pInput->mouseStates[event->button.button] = true;
		break;
	case SDL_MOUSEBUTTONUP:
		pInput->mouseUps[event->button.button] = true;
		pInput->mouseStates[event->button.button] = false;
		break;
	case SDL_MOUSEWHEEL:
		break;
	default:
		break;
	}
}