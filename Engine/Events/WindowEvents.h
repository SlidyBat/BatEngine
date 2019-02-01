#pragma once

#include "BatWinAPI.h"
#include "Event.h"

namespace Bat
{
	// Called when window is resized
	struct WindowResizeEvent
	{
		// New window width
		int width;
		// New window height
		int height;
	};

	// Called when window is moved
	struct WindowMovedEvent
	{
		// X position of window in screen coordinates
		int x;
		// Y position of window in screen coordinates
		int y;
	};


	// Called when window is closed
	struct WindowClosedEvent
	{};
}