#pragma once

#include "Entity.h"

#include "MeshAnimator.h"

namespace Bat
{
	class AnimationComponent : public Component<AnimationComponent>
	{
	public:
		AnimationComponent( MeshAnimator* animator )
			:
			m_pAnimator( animator )
		{}

		void AddChannel( const std::string& anim_name, size_t channel_idx )
		{
			m_Channels.emplace_back( anim_name, channel_idx );
		}

		const AnimationChannel* GetChannel( const std::string& name )
		{
			for( const AnimationNameAndChannel& channel : m_Channels )
			{
				if( channel.name == name )
				{
					const AnimationClip& curr_anim = m_pAnimator->GetCurrentAnimation();
					return &curr_anim.channels[channel.index];
				}
			}

			return nullptr;
		}

		const AnimationChannel* GetCurrentChannel()
		{
			const AnimationClip& curr_anim = m_pAnimator->GetCurrentAnimation();
			return GetChannel( curr_anim.name );
		}

		BoneTransform GetSample()
		{
			const AnimationChannel* channel = GetCurrentChannel();
			float timestamp = m_pAnimator->GetTimestamp();
			return channel->GetSample( timestamp );
		}

		MeshAnimator* GetAnimator() { return m_pAnimator; }
	private:
		struct AnimationNameAndChannel
		{
			AnimationNameAndChannel( const std::string& name, size_t channel_idx )
				:
				name( name ),
				index( channel_idx )
			{}

			std::string name;
			size_t index;
		};
		std::vector<AnimationNameAndChannel> m_Channels;

		MeshAnimator* m_pAnimator;
	};
}