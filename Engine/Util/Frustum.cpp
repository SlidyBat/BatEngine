#include "PCH.h"
#include "Frustum.h"

namespace Bat
{
	Frustum::Frustum( DirectX::XMMATRIX transform )
	{
		transform = DirectX::XMMatrixTranspose( transform );

		const auto l = DirectX::XMPlaneNormalizeEst( DirectX::XMVectorAdd( transform.r[3], transform.r[0] ) );
		DirectX::XMStoreFloat4( &planes[LEFT_PLANE], l );

		const auto r = DirectX::XMPlaneNormalizeEst( DirectX::XMVectorSubtract( transform.r[3], transform.r[0] ) );
		DirectX::XMStoreFloat4( &planes[RIGHT_PLANE], r );

		const auto b = DirectX::XMPlaneNormalizeEst( DirectX::XMVectorAdd( transform.r[3], transform.r[1] ) );
		DirectX::XMStoreFloat4( &planes[BOTTOM_PLANE], b );

		const auto t = DirectX::XMPlaneNormalizeEst( DirectX::XMVectorSubtract( transform.r[3], transform.r[1] ) );
		DirectX::XMStoreFloat4( &planes[TOP_PLANE], t );

		const auto n = DirectX::XMPlaneNormalizeEst( DirectX::XMVectorAdd( transform.r[3], transform.r[2] ) );
		DirectX::XMStoreFloat4( &planes[NEAR_PLANE], n );

		const auto f = DirectX::XMPlaneNormalizeEst( DirectX::XMVectorSubtract( transform.r[3], transform.r[2] ) );
		DirectX::XMStoreFloat4( &planes[FAR_PLANE], f );
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

	Frustum Frustum::Transform( const Frustum& frustum, DirectX::XMMATRIX transform )
	{
		Frustum transformed;
		for( int i = 0; i < TOTAL_PLANES; i++ )
		{
			transformed.planes[i] = DirectX::XMPlaneTransform( frustum.planes[i], transform );
		}

		return transformed;
	}

	void Frustum::CalculateCorners( DirectX::XMMATRIX viewproj, Vec3 corners_out[8] )
	{
		DirectX::XMMATRIX inv_vp = DirectX::XMMatrixInverse( nullptr, viewproj );

		// Corners in homogeneous clip space
		DirectX::XMVECTOR corners[8] =
		{                                                     //                   7--------6
			DirectX::XMVectorSet(  1.0f, -1.0f, 0.0f, 1.0f ), //                  /|       /|
			DirectX::XMVectorSet( -1.0f, -1.0f, 0.0f, 1.0f ), //   Y ^           / |      / |
			DirectX::XMVectorSet(  1.0f,  1.0f, 0.0f, 1.0f ), //   | _          3--------2  |
			DirectX::XMVectorSet( -1.0f,  1.0f, 0.0f, 1.0f ), //   | /' Z       |  |     |  |
			DirectX::XMVectorSet(  1.0f, -1.0f, 1.0f, 1.0f ), //   |/           |  5-----|--4
			DirectX::XMVectorSet( -1.0f, -1.0f, 1.0f, 1.0f ), //   + ---> X     | /      | /
			DirectX::XMVectorSet(  1.0f,  1.0f, 1.0f, 1.0f ), //                |/       |/
			DirectX::XMVectorSet( -1.0f,  1.0f, 1.0f, 1.0f ), //                1--------0
		};

		// Convert to world space
		for( int i = 0; i < 8; ++i )
		{
			corners_out[i] = DirectX::XMVector3TransformCoord( corners[i], inv_vp );
		}
	}

	float Frustum::PlaneDotCoord( const Vec4& plane, const Vec3& point )
	{
		DirectX::XMVECTOR p = DirectX::XMLoadFloat4( &plane );
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3( &point );

		float d;
		DirectX::XMStoreFloat( &d, DirectX::XMPlaneDotCoord( p, v ) );

		return d;
	}
}
