#pragma once

#include "Common.h"

namespace Bat
{
	enum class RenderFlags
	{
		NONE = 0,
		DRAW_BBOX, // Draws a bounding box around the entity's model
		INSTANCED // Draws all instances of mesh in a single draw call
	};
	BAT_ENUM_OPERATORS( RenderFlags );
}