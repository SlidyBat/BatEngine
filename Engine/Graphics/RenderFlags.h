#pragma once

#include "Common.h"

namespace Bat
{
	enum class RenderFlags
	{
		NONE = 0,
		DRAW_BBOX // Draws a bounding box around the entity's model
	};
	BAT_ENUM_OPERATORS( RenderFlags );
}