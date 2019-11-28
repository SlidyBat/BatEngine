#pragma once

#include "MathLib.h"

namespace Bat
{
	template <typename T>
	struct KeyFrame
	{
		// Bone space transform for the animated bone
		T value;
		// Timestamp in seconds in the animation
		float timestamp;

		bool operator==( const KeyFrame& rhs )
		{
			return value == rhs.value;
		}
		bool operator!=( const KeyFrame& rhs )
		{
			return value != rhs.value;
		}
		bool operator<( const KeyFrame& rhs )
		{
			return timestamp < rhs.timestamp;
		}
		bool operator<=( const KeyFrame& rhs )
		{
			return timestamp <= rhs.timestamp;
		}
		bool operator>( const KeyFrame& rhs )
		{
			return timestamp > rhs.timestamp;
		}
		bool operator>=( const KeyFrame& rhs )
		{
			return timestamp >= rhs.timestamp;
		}
	};

	using PosKeyFrame = KeyFrame<Vec3>; // Value is position in bone space
	using RotKeyFrame = KeyFrame<Vec4>; // Value is rotation in bone space as a quaternion
}