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

		for( int i = 0; i < 6; i++ )
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
		for( int i = 0; i < 6; i++ )
		{
			if( PlaneDotCoord( planes[i], centre ) < -radius ) return false;
		}

		return true;
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
