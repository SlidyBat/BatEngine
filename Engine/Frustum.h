#pragma once

#include "MathLib.h"

namespace Bat
{
	enum PlaneId
	{
		LEFT_PLANE,
		RIGHT_PLANE,
		BOTTOM_PLANE,
		TOP_PLANE,
		NEAR_PLANE,
		FAR_PLANE,
		TOTAL_PLANES
	};
	class Frustum
	{
	public:
		Frustum() = default;
		Frustum( DirectX::XMMATRIX transform );
		
		bool IsPointInside( const Vec3& point ) const;
		bool IsBoxInside( const Vec3& mins, const Vec3& maxs ) const;
		bool IsSphereInside( const Vec3& centre, float radius ) const;
	private:
		// Does a dot product between plane and coord using SIMD magic
		static float PlaneDotCoord( const Vec4& plane, const Vec3& point );
	public:
		Vec4 planes[TOTAL_PLANES];
	};
}