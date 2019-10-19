#include "PCH.h"
#include "EntityTrace.h"

namespace Bat
{
	static Entity PhysicsObjectToEntity( IPhysicsObject* obj );
	static RayCastResult PhysicsRayCastToRayCastResult( const PhysicsRayCastResult& phys_result );
	static SweepResult PhysicsSweepToSweepResult( const PhysicsSweepResult& phys_result );

	RayCastResult EntityTrace::RayCast( const Vec3& origin, const Vec3& unit_direction, float max_distance, int filter )
	{
		PhysicsRayCastResult phys_result = Physics::RayCast( origin, unit_direction, max_distance, filter );
		return PhysicsRayCastToRayCastResult( phys_result );
	}
	SweepResult EntityTrace::SweepSphere( float radius, const Vec3& origin, const Vec3& unit_direction, float max_distance, int filter )
	{
		PhysicsSweepResult phys_result = Physics::SweepSphere( radius, origin, unit_direction, max_distance, filter );
		return PhysicsSweepToSweepResult( phys_result );
	}
	SweepResult EntityTrace::SweepCapsule( float radius, float half_height, const Vec3& rotation, const Vec3& origin, const Vec3& unit_direction, float max_distance, int filter )
	{
		PhysicsSweepResult phys_result = Physics::SweepCapsule( radius, half_height, origin, rotation, unit_direction, max_distance, filter );
		return PhysicsSweepToSweepResult( phys_result );
	}
	SweepResult EntityTrace::SweepBox( float length_x, float length_y, float length_z, const Vec3& rotation, const Vec3& origin, const Vec3& unit_direction, float max_distance, int filter )
	{
		PhysicsSweepResult phys_result = Physics::SweepBox( length_x, length_y, length_z, rotation, origin, unit_direction, max_distance, filter );
		return PhysicsSweepToSweepResult( phys_result );
	}

	static Entity PhysicsObjectToEntity( IPhysicsObject* obj )
	{
		return Entity( world, Entity::Id( (uint64_t)obj->GetUserData() ) );
	}
	static RayCastResult PhysicsRayCastToRayCastResult( const PhysicsRayCastResult& phys_result )
	{
		RayCastResult result;
		result.hit = phys_result.hit;
		result.position = phys_result.position;
		result.normal = phys_result.normal;
		result.distance = phys_result.distance;

		if( phys_result.hit )
		{
			result.entity = PhysicsObjectToEntity( phys_result.object );
		}
		else
		{
			result.entity = Entity::INVALID;
		}

		return result;
	}
	static SweepResult PhysicsSweepToSweepResult( const PhysicsSweepResult& phys_result )
	{
		SweepResult result;
		result.hit = phys_result.hit;
		result.position = phys_result.position;
		result.distance = phys_result.distance;

		if( phys_result.hit )
		{
			result.entity = PhysicsObjectToEntity( phys_result.object );
		}
		else
		{
			result.entity = Entity::INVALID;
		}

		return result;
	}
}
