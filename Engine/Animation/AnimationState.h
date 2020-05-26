#pragma once

#include "AnimationSkeleton.h"
#include "AnimationClip.h"
#include "Graphics/IGPUDevice.h"
#include "Graphics/ConstantBuffer.h"

namespace Bat
{
	class AnimationState
	{
	public:
		AnimationState( AnimationClip* clip );

		const AnimationClip* GetClip() const { return m_pClip; }

		float GetTimestamp() const { return m_flTimestamp; }
		void SetTimestamp( float timestamp ) { m_flTimestamp = timestamp; FixTimestampRange(); }

		float GetTimescale() const { return m_flTimescale; }
		void SetTimescale( float timescale ) { m_flTimescale = timescale; }

		float GetWeight() const { return m_flWeight; }
		void SetWeight( float weight ) { m_flWeight = weight; }

		bool IsLooping() const { return m_bLooping; }
		void SetLooping( bool looping ) { m_bLooping = looping; }

		bool IsActive() const { return m_bActive; }
		void SetActive( bool active ) { m_bActive = active; }

		void Update( float dt );
		SkeletonPose GetSample( const SkeletonPose& bind_pose ) const;

		void DoImGuiMenu();
	private:
		void FixTimestampRange();
	private:
		AnimationClip* m_pClip = nullptr;
		float m_flTimestamp = 0.0f;
		float m_flTimescale = 1.0f;
		float m_flWeight = 1.0f;
		bool m_bLooping = false;
		bool m_bActive = true;
	};
}