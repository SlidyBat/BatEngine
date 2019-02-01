#pragma once

#include "Event.h"

namespace Bat
{
	// Called when a key is pressed
	struct KeyPressedEvent
	{
		// Virtual key code of key that was pressed
		size_t key;
		// Whether this is a repeated key
		bool repeated;
	};

	// Called when a key is released
	struct KeyReleasedEvent
	{
		// Virtual key code of key that was pressed
		size_t key;
	};
}