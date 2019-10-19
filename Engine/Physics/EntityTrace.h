#pragma once

#include "MathLib.h"
#include "Entity.h"
#include "Physics.h"

namespace Bat
{
	struct RayCastResult
	{
		bool hit; // Whether or not there was a hit. If false, other data in the result is invalid.
		Vec3 position;
		Vec3 normal;
		float distance;
		Entity entity;
	};

	struct SweepResult
	{
		bool hit; // Whether or not there was a hit. If false, other data in the result is invalid.
		Vec3 position;
		float distance;
		Entity entity;
	};

	class EntityTrace
	{
	public:
		// See TraceFilterFlags for possible filter flags
		static RayCastResult RayCast( const Vec3& origin, const Vec3& unit_direction, float max_distance, int filter = (HIT_STATICS | HIT_DYNAMICS) );
		static SweepResult SweepSphere( float radius, const Vec3& origin, const Vec3& unit_direction, float max_distance, int filter = (HIT_STATICS | HIT_DYNAMICS) );
		static SweepResult SweepCapsule( float radius, float half_height, const Vec3& rotation, const Vec3& origin, const Vec3& unit_direction, float max_distance, int filter = (HIT_STATICS | HIT_DYNAMICS) );
		static SweepResult SweepBox( float length_x, float length_y, float length_z, const Vec3& rotation, const Vec3& origin, const Vec3& unit_direction, float max_distance, int filter = (HIT_STATICS | HIT_DYNAMICS) );
	};
}