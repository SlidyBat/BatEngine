#include "PCH.h"
#include "Frustum.h"

namespace Bat
{
	Frustum::Frustum( const Mat4& transform )
	{
		planes[LEFT_PLANE]   = Plane::Normalize( transform.GetRow( 3 ) + transform.GetRow( 0 ) );
		planes[RIGHT_PLANE]  = Plane::Normalize( transform.GetRow( 3 ) - transform.GetRow( 0 ) );
		planes[BOTTOM_PLANE] = Plane::Normalize( transform.GetRow( 3 ) + transform.GetRow( 1 ) );
		planes[TOP_PLANE]    = Plane::Normalize( transform.GetRow( 3 ) - transform.GetRow( 1 ) );
		planes[NEAR_PLANE]   = Plane::Normalize( transform.GetRow( 3 ) + transform.GetRow( 2 ) );
		planes[FAR_PLANE]    = Plane::Normalize( transform.GetRow( 3 ) - transform.GetRow( 2 ) );
	}

	bool Frustum::IsPointInside( const Vec3& point ) const
	{
		for( int i = 0; i < TOTAL_PLANES; i++ )
		{
			if( PlaneDotCoord( planes[i], point ) < 0.0f ) return false;
		}

		return true;
	}

	bool Frustum::IsBoxInside( const Vec3& mins, const Vec3& maxs ) const
	{
		Vec3 centre = (mins + maxs) * 0.5f;
		Vec3 extents = (maxs - mins) * 0.5f;

		for( int i = 0; i < TOTAL_PLANES; i++ )
		{
			if( PlaneDotCoord( planes[i], { centre.x - extents.x, centre.y - extents.y, centre.z - extents.z } ) >= 0.0f )
			{
				continue;
			}
			if( PlaneDotCoord( planes[i], { centre.x + extents.x, centre.y - extents.y, centre.z - extents.z } ) >= 0.0f )
			{
				continue;
			}
			if( PlaneDotCoord( planes[i], { centre.x - extents.x, centre.y + extents.y, centre.z - extents.z } ) >= 0.0f )
			{
				continue;
			}
			if( PlaneDotCoord( planes[i], { centre.x - extents.x, centre.y - extents.y, centre.z + extents.z } ) >= 0.0f )
			{
				continue;
			}
			if( PlaneDotCoord( planes[i], { centre.x - extents.x, centre.y + extents.y, centre.z + extents.z } ) >= 0.0f )
			{
				continue;
			}
			if( PlaneDotCoord( planes[i], { centre.x + extents.x, centre.y - extents.y, centre.z + extents.z } ) >= 0.0f )
			{
				continue;
			}
			if( PlaneDotCoord( planes[i], { centre.x + extents.x, centre.y + extents.y, centre.z - extents.z } ) >= 0.0f )
			{
				continue;
			}
			if( PlaneDotCoord( planes[i], { centre.x + extents.x, centre.y + extents.y, centre.z + extents.z } ) >= 0.0f )
			{
				continue;
			}

			return false;
		}

		return true;
	}

	bool Frustum::IsSphereInside( const Vec3& centre, float radius ) const
	{
		for( int i = 0; i < TOTAL_PLANES; i++ )
		{
			if( PlaneDotCoord( planes[i], centre ) < -radius ) return false;
		}

		return true;
	}

	Frustum Frustum::Transform( const Frustum& frustum, const Mat4& transform )
	{
		Frustum transformed;
		for( int i = 0; i < TOTAL_PLANES; i++ )
		{
			transformed.planes[i] = transform * frustum.planes[i];
		}

		return transformed;
	}

	void Frustum::CalculateCorners( const Mat4& viewproj, Vec3 corners_out[8] )
	{
		Mat4 inv_vp = Mat4::Inverse( viewproj );

		// Corners in homogeneous clip space
		Vec3 corners[8] =
		{                           //                   7--------6
			{  1.0f, -1.0f, 0.0f }, //                  /|       /|
			{ -1.0f, -1.0f, 0.0f }, //   Y ^           / |      / |
			{  1.0f,  1.0f, 0.0f }, //   | _          3--------2  |
			{ -1.0f,  1.0f, 0.0f }, //   | /' Z       |  |     |  |
			{  1.0f, -1.0f, 1.0f }, //   |/           |  5-----|--4
			{ -1.0f, -1.0f, 1.0f }, //   + ---> X     | /      | /
			{  1.0f,  1.0f, 1.0f }, //                |/       |/
			{ -1.0f,  1.0f, 1.0f }, //                1--------0
		};

		// Convert to world space
		for( int i = 0; i < 8; ++i )
		{
			corners_out[i] = inv_vp * corners[i];
		}
	}

	float Frustum::PlaneDotCoord( const Plane& plane, const Vec3& point )
	{
		return Vec3::Dot( plane.n, point ) + plane.d;
	}
}
