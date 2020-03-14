#pragma once

#include "MathLib.h"

namespace Bat
{
	class Frustum
	{
	public:
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
	public:
		Frustum() = default;
		Frustum( const Mat4& transform );
		
		bool IsPointInside( const Vec3& point ) const;
		bool IsBoxInside( const Vec3& mins, const Vec3& maxs ) const;
		bool IsSphereInside( const Vec3& centre, float radius ) const;

		static Frustum Transform( const Frustum& frustum, const Mat4& transform );
		static void CalculateCorners( const Mat4& transform, Vec3 corners_out[8] );
	private:
		// Does a dot product between plane and coord using SIMD magic
		static float PlaneDotCoord( const Plane& plane, const Vec3& point );
	public:
		Plane planes[TOTAL_PLANES];
	};
}