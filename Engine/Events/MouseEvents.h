#pragma once

#include "Event.h"
#include "Util/MathLib.h"
#include "Platform/Input.h"

namespace Bat
{
	// Called when mouse moves
	struct MouseMovedEvent
	{
		Vei2 GetDeltaPosition() const { return pos - old_pos; }

		// New mouse position
		Vei2 pos;
		// Mouse position before it was moved
		Vei2 old_pos;
	};

	// Called when mouse wheel is scrolled
	struct MouseScrolledEvent
	{
		// Mouse position when scroll wheel moved
		Vei2 pos;
		// How much the scroll wheel was moved
		float delta;
	};

	// Called when a mouse button is pressed
	struct MouseButtonPressedEvent
	{
		// Mouse position when button was pressed
		Vei2 pos;
		// Mouse button that was pressed
		Input::MouseButton button;
	};

	// Called when a mouse button is released
	struct MouseButtonReleasedEvent
	{
		// Mouse position when button was released
		Vei2 pos;
		// Mouse button that was released
		Input::MouseButton button;
	};

	// Called when a mouse button 
	struct MouseButtonDoubleClickEvent
	{
		// Mouse position when button was double clicked
		Vei2 pos;
		// Mouse button that was double clicked
		Input::MouseButton button;
	};

	// Called when mouse enters window client area
	struct MouseEnterEvent {};

	// Called when mouse exits window client area
	struct MouseLeaveEvent {};
}