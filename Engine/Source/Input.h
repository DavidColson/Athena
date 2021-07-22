#pragma once

#include <EASTL/bitset.h>
#include <Core/Vec2.h>

struct Scene;
union SDL_Event;

namespace Input
{
	void CreateInputState();

	bool GetKeyDown(int keyCode);
	bool GetKeyUp(int keyCode);
	bool GetKeyHeld(int keyCode);

	bool GetMouseDown(int buttonCode);
	bool GetMouseUp(int buttonCode);
	bool GetMouseHeld(int buttonCode);

	Vec2f GetMouseDelta();
	bool GetMouseInRelativeMode();

	void ClearState();
	void ClearHeldState();
	void ProcessEvent(SDL_Event* event);
};