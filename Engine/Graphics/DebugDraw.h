#pragma once

#include "MathLib.h"
#include "Colour.h"

namespace Bat
{
	class IGPUContext;
	class Camera;

	class DebugDraw
	{
	public:
		// Draws a rectangle at given screen space coordinates
		static void Rectangle( const Vei2& a, const Vei2& b, const Colour& col = Colours::White, float thickness = 5.0f );
		// Draws a box at given world space coordinates
		static void Box( const Vec3& a, const Vec3& b, const Colour& col = Colours::White );
		// Draws a line at given world space coordinates
		static void Line( const Vec3& a, const Vec3& b, const Colour& col = Colours::White );
		static void Text( const std::string& str, const Vei2& pos, const Colour& col = Colours::White );

		static void Flush( const Camera& camera );
	};
}