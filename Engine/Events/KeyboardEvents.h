#pragma once

#include "Event.h"

namespace Bat
{
	struct KeyPressedEvent
	{
		size_t key;
		bool repeated;
	};

	struct KeyReleasedEvent
	{
		size_t key;
	};
}