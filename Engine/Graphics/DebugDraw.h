#pragma once

#include "MathLib.h"
#include "Colour.h"

namespace Bat
{
	class IGPUContext;

	class DebugDraw
	{
	public:
		static void Rectangle( const Vei2& a, const Vei2& b, const Colour& col = Colours::White, float thickness = 5.0f );
		static void Text( const std::string& str, const Vei2& pos, const Colour& col = Colours::White );

		static void Flush();
	};
}