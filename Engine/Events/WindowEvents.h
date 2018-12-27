#pragma once

#include "BatWinAPI.h"
#include "Event.h"

namespace Bat
{
	struct WindowResizeEvent
	{
		// New window width
		int width;
		// New window height
		int height;
	};

	struct WindowMovedEvent
	{
		// X position of window in screen coordinates
		int x;
		// Y position of window in screen coordinates
		int y;
	};

	struct WindowClosedEvent
	{};
}