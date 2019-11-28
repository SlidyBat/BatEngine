#include "PCH.h"
#include "AnimationChannel.h"

namespace Bat
{
	template <typename T>
	static void GetPrevAndNextKeyFrames( const std::vector<KeyFrame<T>>& keyframes, float timestamp, size_t start_at, size_t* pPrevKeyFrame, size_t* pNextKeyFrame )
	{
		*pPrevKeyFrame = start_at;
		for( size_t i = start_at + 1; i < keyframes.size(); i++ )
		{
			*pNextKeyFrame = i;
			if( keyframes[*pNextKeyFrame].timestamp > timestamp )
			{
				break;
			}

			*pPrevKeyFrame = *pNextKeyFrame;
		}
	}

	static Vec3 InterpolatePosKeyFrames( const PosKeyFrame& prev_key, const PosKeyFrame& next_key, float timestamp )
	{
		const float range = next_key.timestamp - prev_key.timestamp;
		const float pct = (timestamp - prev_key.timestamp) / range;
		return next_key.value * pct + prev_key.value * (1 - pct);
	}

	static Vec3 GetPosKeyFrameAt( const std::vector<PosKeyFrame>& keyframes, size_t* last_pos_index, float timestamp )
	{
		// No interpolation needed for only 1 keyframe
		if( keyframes.size() == 1 )
		{
			return keyframes[0].value;
		}

		// If timestamp is outside of bounds of keyframes, clamp it
		if( timestamp <= keyframes.front().timestamp )
		{
			return keyframes.front().value;
		}
		if( timestamp >= keyframes.back().timestamp )
		{
			return keyframes.back().value;
		}

		size_t prev_key_frame;
		size_t next_key_frame;
		GetPrevAndNextKeyFrames( keyframes, timestamp, *last_pos_index, &prev_key_frame, &next_key_frame );
		*last_pos_index = prev_key_frame;
		return InterpolatePosKeyFrames( keyframes[prev_key_frame], keyframes[next_key_frame], timestamp );
	}

	static Vec4 InterpolateRotKeyFrames( const RotKeyFrame& prev_key, const RotKeyFrame& next_key, float timestamp )
	{
		const float range = next_key.timestamp - prev_key.timestamp;
		const float pct = (timestamp - prev_key.timestamp) / range;

		return DirectX::XMQuaternionSlerp( prev_key.value, next_key.value, pct );
	}

	static Vec4 GetRotKeyFrameAt( const std::vector<RotKeyFrame>& keyframes, size_t* last_rot_index, float timestamp )
	{
		if( keyframes.size() == 1 )
		{
			return keyframes[0].value;
		}

		size_t prev_key_frame;
		size_t next_key_frame;
		GetPrevAndNextKeyFrames( keyframes, timestamp, *last_rot_index, &prev_key_frame, &next_key_frame );
		*last_rot_index = prev_key_frame;
		return InterpolateRotKeyFrames( keyframes[prev_key_frame], keyframes[next_key_frame], timestamp );
	}

	BoneTransform AnimationChannel::GetSample( float timestamp ) const
	{
		if( timestamp < last_timestamp )
		{
			ResetCache();
		}
		last_timestamp = timestamp;

		// No channels case
		if( position_keyframes.empty() || rotation_keyframes.empty() )
		{
			ASSERT( position_keyframes.empty() && rotation_keyframes.empty(), "Only 1 keyframe type provided" );
			return {};
		}

		BoneTransform transform;
		transform.translation = GetPosKeyFrameAt( position_keyframes, &last_pos_index, timestamp );
		transform.rotation = GetRotKeyFrameAt( rotation_keyframes, &last_rot_index, timestamp );
		return transform;
	}

	void AnimationChannel::ResetCache() const
	{
		last_pos_index = 0;
		last_rot_index = 0;
	}
}