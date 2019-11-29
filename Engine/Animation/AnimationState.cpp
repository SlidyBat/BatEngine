#include "PCH.h"
#include "AnimationState.h"

#include "imgui.h"

namespace Bat
{
	AnimationState::AnimationState( AnimationClip* clip )
		:
		m_pClip( clip )
	{}
	void AnimationState::Update( float dt )
	{
		if( m_bActive )
		{
			m_flTimestamp += dt * m_flTimescale;
			FixTimestampRange();
		}
	}
	SkeletonPose AnimationState::GetSample( const SkeletonPose& bind_pose ) const
	{
		return m_pClip->GetSample( m_flTimestamp, bind_pose );
	}
	void AnimationState::DoImGuiMenu()
	{
		if( ImGui::TreeNode( m_pClip->name.c_str() ) )
		{
			ImGui::SliderFloat( "Timestamp", &m_flTimestamp, 0.0f, m_pClip->duration );
			ImGui::DragFloat( "Timescale", &m_flTimescale, 0.05f, -5.0f, 5.0f );
			ImGui::DragFloat( "Weight", &m_flWeight, 0.01f, 0.0f, 1.0f );
			ImGui::Checkbox( "Loop", &m_bLooping );
			ImGui::Checkbox( "Active", &m_bActive );

			ImGui::TreePop();
		}
	}
	void AnimationState::FixTimestampRange()
	{
		if( m_bLooping )
		{
			while( m_flTimestamp >= m_pClip->duration )
			{
				m_flTimestamp -= m_pClip->duration;
			}
			while( m_flTimestamp < 0.0f )
			{
				m_flTimestamp += m_pClip->duration;
			}
		}
		else
		{
			m_flTimestamp = std::clamp( m_flTimestamp, 0.0f, m_pClip->duration - 0.01f );
		}
	}
}
