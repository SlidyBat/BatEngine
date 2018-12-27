#pragma once

#include "Event.h"
#include "MathLib.h"
#include "Input.h"

namespace Bat
{
	struct MouseMovedEvent
	{
		Vei2 GetDeltaPosition() const { return pos - old_pos; }

		// New mouse position
		Vei2 pos;
		// Mouse position before it was moved
		Vei2 old_pos;
	};

	struct MouseScrolledEvent
	{
		// Mouse position when button was released
		Vei2 pos;
		// How much the scroll wheel was moved
		float delta;
	};

	struct MouseButtonPressedEvent
	{
		// Mouse position when button was released
		Vei2 pos;
		// Mouse button that was pressed
		Input::MouseButton button;
	};

	struct MouseButtonReleasedEvent
	{
		// Mouse position when button was released
		Vei2 pos;
		// Mouse button that was released
		Input::MouseButton button;
	};

	struct MouseButtonDoubleClickEvent
	{
		// Mouse position when button was released
		Vei2 pos;
		// Mouse button that was double clicked
		Input::MouseButton button;
	};
}