#pragma once

#include <string>
#include <vector>
#include "AnimationSkeleton.h"
#include "AnimationChannel.h"

namespace Bat
{
	struct AnimationClip
	{
		std::string name;
		std::vector<AnimationChannel> channels;
		float duration;

		// Returns bone space transforms for each node in the skeleton at the given timestamp
		SkeletonPose GetSample( float timestamp, const SkeletonPose& bind_pose ) const;
	};
}