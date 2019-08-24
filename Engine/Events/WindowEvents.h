#pragma once

#include "BatWinAPI.h"
#include "Event.h"

namespace Bat
{
	// Called when window is resized
	struct WindowResizeEvent
	{
		// New window width
		size_t width;
		// New window height
		size_t height;
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