#include "PCH.h"
#include "AnimationClip.h"

namespace Bat
{
	SkeletonPose AnimationClip::GetSample( float timestamp, const SkeletonPose& bind_pose ) const
	{
		SkeletonPose new_pose = bind_pose;

		for( const AnimationChannel& channel : channels )
		{
			new_pose.bones[channel.node_index].transform = channel.GetSample( timestamp );
		}

		return new_pose;
	}
}