#pragma once

#include "Event.h"

namespace Bat
{
	// Called when a character is typed
	struct KeyCharEvent
	{
		// Virtual key code of key that was pressed
		size_t key;
		// Whether this is a repeated key
		bool repeated;
	};

	// Called when a key is pressed
	// Unlike KeyCharEvent, the key pressed is always represented the same
	// regardless of modifiers. e.g. if W key is pressed it'll always show
	// up as 'W' and not 'w'.
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