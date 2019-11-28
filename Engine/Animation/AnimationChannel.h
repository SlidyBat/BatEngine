#pragma once

#include <vector>
#include "AnimationSkeleton.h"
#include "KeyFrame.h"

namespace Bat
{
	class AnimationChannel
	{
	public:
		// Returns bone space transform of this bone in the pose at the given timestamp
		BoneTransform GetSample( float timestamp ) const;
	public:
		std::vector<PosKeyFrame> position_keyframes;
		std::vector<RotKeyFrame> rotation_keyframes;
		int node_index;
	private:
		void ResetCache() const;
	private:
		mutable size_t last_pos_index = 0;
		mutable size_t last_rot_index = 0;
		mutable float last_timestamp = 0.0f;
	};
}