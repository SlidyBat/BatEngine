#pragma once

#include "AnimationSkeleton.h"
#include "AnimationClip.h"
#include "IGPUDevice.h"
#include "ConstantBuffer.h"

namespace Bat
{
	class MeshAnimator
	{
	public:
		MeshAnimator() = default;
		MeshAnimator( SkeletonPose bind_pose, std::vector<BoneData> bones, std::vector<AnimationClip> animations )
			:
			m_BindPose( std::move( bind_pose ) ),
			m_Bones( std::move( bones ) ),
			m_Animations( std::move( animations ) )
		{}

		void Bind( IGPUContext* pContext );

		size_t GetNumAnimations() const { return m_Animations.size(); }
		const AnimationClip& GetAnimation( size_t index ) const { return m_Animations[index]; }
		const AnimationClip* GetAnimationByName( const std::string& name ) const;

		size_t GetCurrentAnimationIndex() const { return m_iCurrentAnimation; }
		void SetCurrentAnimationIndex( size_t index ) { m_iCurrentAnimation = index; }
		const AnimationClip& GetCurrentAnimation() const { return GetAnimation( GetCurrentAnimationIndex() ); }

		float GetTimestamp() const { return m_flTimestamp; }
		void SetTimestamp( float timestamp ) { m_flTimestamp = timestamp; }
	private:
		void GetMatrixPalette( DirectX::XMMATRIX* out, const AnimationClip& animation, float timestamp ) const;
	private:
		// Vector of each bone, guaranteed to have parent pones positioned before their child bones
		std::vector<BoneData> m_Bones;
		SkeletonPose m_BindPose;
		std::vector<AnimationClip> m_Animations;
	
		struct CB_Bones
		{
			DirectX::XMMATRIX bone_transforms[MAX_BONES];
		};
		ConstantBuffer<CB_Bones> m_cbufBones;

		size_t m_iCurrentAnimation = 0;
		float m_flTimestamp = 0.0f;
	};
}