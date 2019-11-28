#include "PCH.h"
#include "MeshAnimator.h"

#include "CoreEntityComponents.h"
#include "PhysicsComponent.h"
#include "ShaderManager.h"

namespace Bat
{
	void MeshAnimator::Bind( IGPUContext* pContext )
	{
		const AnimationClip& animation = GetAnimation( GetCurrentAnimationIndex() );

		// TODO: Share cbuf between all animators
		DirectX::XMMATRIX* cbuf = m_cbufBones.Lock( pContext )->bone_transforms;

		GetMatrixPalette( cbuf, animation, GetTimestamp() );

		m_cbufBones.Unlock( pContext );

		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufBones, VS_CBUF_BONES );
	}

	const AnimationClip* MeshAnimator::GetAnimationByName( const std::string& name ) const
	{
		for( const AnimationClip& animation : m_Animations )
		{
			if( animation.name == name )
			{
				return &animation;
			}
		}

		return nullptr;
	}

	void MeshAnimator::GetMatrixPalette( DirectX::XMMATRIX* out, const AnimationClip& animation, float timestamp ) const
	{
		ASSERT( m_Bones.size() <= MAX_BONES, "Too many bones!" );

		SkeletonPose pose = animation.GetSample( timestamp, m_BindPose );
		auto model_pose = SkeletonPose::ToModelSpace( std::move( pose ) );
		SkeletonPose::ToMatrixPalette( model_pose, m_Bones, out );
	}
}
