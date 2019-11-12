#include "PCH.h"
#include "MeshAnimation.h"

#include "CoreEntityComponents.h"
#include "PhysicsComponent.h"
#include "ShaderManager.h"

namespace Bat
{
	template <typename T>
	static void GetPrevAndNextKeyFrames( const std::vector<KeyFrame<T>>& keyframes, float timestamp, const KeyFrame<T>** ppPrevKeyFrame, const KeyFrame<T>** ppNextKeyFrame )
	{
		*ppPrevKeyFrame = &keyframes[0];
		for( size_t i = 1; i < keyframes.size(); i++ )
		{
			*ppNextKeyFrame = &keyframes[i];
			if( (*ppNextKeyFrame)->timestamp > timestamp )
			{
				break;
			}

			*ppPrevKeyFrame = *ppNextKeyFrame;
		}
	}

	static Vec3 InterpolatePosKeyFrames( const PosKeyFrame& prev_key, const PosKeyFrame& next_key, float timestamp )
	{
		const float range = next_key.timestamp - prev_key.timestamp;
		const float pct = (timestamp - prev_key.timestamp) / range;
		return next_key.value * pct + prev_key.value * (1 - pct);
	}

	static Vec3 GetPosKeyFrameAt( const std::vector<PosKeyFrame>& keyframes, float timestamp )
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

		const PosKeyFrame* pPrevKeyFrame;
		const PosKeyFrame* pNextKeyFrame;
		GetPrevAndNextKeyFrames( keyframes, timestamp, &pPrevKeyFrame, &pNextKeyFrame );
		return InterpolatePosKeyFrames( *pPrevKeyFrame, *pNextKeyFrame, timestamp );
	}

	static Vec4 InterpolateRotKeyFrames( const RotKeyFrame& prev_key, const RotKeyFrame& next_key, float timestamp )
	{
		const float range = next_key.timestamp - prev_key.timestamp;
		const float pct = (timestamp - prev_key.timestamp) / range;

		return DirectX::XMQuaternionSlerp( prev_key.value, next_key.value, pct );
	}

	static Vec4 GetRotKeyFrameAt( const std::vector<RotKeyFrame>& keyframes, float timestamp )
	{
		if( keyframes.size() == 1 )
		{
			return keyframes[0].value;
		}

		const RotKeyFrame* pPrevKeyFrame;
		const RotKeyFrame* pNextKeyFrame;
		GetPrevAndNextKeyFrames( keyframes, timestamp, &pPrevKeyFrame, &pNextKeyFrame );
		return InterpolateRotKeyFrames( *pPrevKeyFrame, *pNextKeyFrame, timestamp );
	}

	DirectX::XMMATRIX AnimationChannel::GetSample( float timestamp ) const
	{
		// No channels case
		if( position_keyframes.empty() || rotation_keyframes.empty() )
		{
			ASSERT( position_keyframes.empty() && rotation_keyframes.empty(), "Only 1 keyframe type provided" );
			return DirectX::XMMatrixIdentity();
		}

		Vec3 interp_pos = GetPosKeyFrameAt( position_keyframes, timestamp );
		Vec4 interp_rot = GetRotKeyFrameAt( rotation_keyframes, timestamp );

		return DirectX::XMMatrixRotationQuaternion( interp_rot ) *
			DirectX::XMMatrixTranslation( interp_pos.x, interp_pos.y, interp_pos.z );
	}

	std::vector<DirectX::XMMATRIX> MeshAnimation::GetSample( float timestamp, const std::vector<BoneNode>& original_skeleton ) const
	{
		std::vector<DirectX::XMMATRIX> new_transforms;
		new_transforms.reserve( original_skeleton.size() );
		std::transform( original_skeleton.begin(), original_skeleton.end(), std::back_inserter( new_transforms ), []( const BoneNode& node ) {
			return node.local_transform;
		} );

		for( const AnimationChannel& channel : channels )
		{
			new_transforms[channel.node_index] = channel.GetSample( timestamp );
		}

		return new_transforms;
	}

	void MeshAnimator::Bind( IGPUContext* pContext )
	{
		const MeshAnimation& animation = GetAnimation( GetCurrentAnimationIndex() );

		// TODO: Share cbuf between all animators
		DirectX::XMMATRIX* cbuf = m_cbufBones.Lock( pContext )->bone_transforms;

		GetPoseOffsetTransforms( cbuf, animation, GetTimestamp() );

		m_cbufBones.Unlock( pContext );

		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufBones, VS_CBUF_BONES );
	}

	const MeshAnimation* MeshAnimator::GetAnimationByName( const std::string& name ) const
	{
		for( const MeshAnimation& animation : m_Animations )
		{
			if( animation.name == name )
			{
				return &animation;
			}
		}

		return nullptr;
	}

	void MeshAnimator::GetPoseOffsetTransforms( DirectX::XMMATRIX* out, const MeshAnimation& animation, float timestamp ) const
	{
		ASSERT( m_Bones.size() <= MAX_BONES, "Too many bones!" );

		// Initially given in local space, need to convert to model space by concatenating transforms down skeleton, then multiply by inverse bind of bind pose to get offset
		std::vector<DirectX::XMMATRIX> transforms = animation.GetSample( timestamp, m_OriginalSkeleton );

		for( size_t i = 1; i < transforms.size(); i++ )
		{
			transforms[i] = transforms[i] * transforms[m_OriginalSkeleton[i].parent_index];
		}

		for( size_t i = 0; i < m_Bones.size(); i++ )
		{
			out[i] = m_Bones[i].inverse_bind_transform * transforms[m_Bones[i].index];
		}
	}

	void AnimationSystem::Update( EntityManager& world )
	{
		for( Entity ent : world )
		{
			if( ent.Has<AnimationComponent>() )
			{
				ASSERT( !ent.Has<PhysicsComponent>(), "Physics components interferes with animation component" );

				auto& anim = ent.Get<AnimationComponent>();
				auto& t = ent.Get<TransformComponent>();

				const AnimationChannel* channel = anim.GetCurrentChannel();
				if( !channel )
				{
					continue;
				}

				DirectX::XMMATRIX sample = anim.GetSample();
				t.SetTransform( sample );
			}
		}
	}
}
