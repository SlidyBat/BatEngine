#include "PCH.h"
#include "AnimationSkeleton.h"

namespace Bat
{
	BoneTransform BoneTransform::operator*( const BoneTransform& rhs )
	{
		// Wildly inefficient but trying to concatenate translation/rotation myself wasn't working out
		return FromMatrix( ToMatrix( *this ) * ToMatrix( rhs ) );
	}
	DirectX::XMMATRIX BoneTransform::ToMatrix( const BoneTransform& transform )
	{
		return DirectX::XMMatrixRotationQuaternion( transform.rotation ) *
			DirectX::XMMatrixTranslation( transform.translation.x, transform.translation.y, transform.translation.z );
	}
	BoneTransform BoneTransform::FromMatrix( DirectX::XMMATRIX matrix )
	{
		DirectX::XMVECTOR translation, rotation, scale;
		DirectX::XMMatrixDecompose( &scale, &rotation, &translation, matrix );

		BoneTransform transform;
		transform.translation = translation;
		transform.rotation = rotation;

		return transform;
	}
	SkeletonPose SkeletonPose::ToModelSpace( SkeletonPose pose )
	{
		for( size_t i = 1; i < pose.bones.size(); i++ )
		{
			int parent_index = pose.bones[i].parent_index;
			pose.bones[i].transform = pose.bones[i].transform * pose.bones[parent_index].transform;
		}

		return pose;
	}
	void SkeletonPose::ToMatrixPalette( const SkeletonPose& model_pose, const std::vector<BoneData>& bones, DirectX::XMMATRIX* out )
	{
		for( size_t i = 0; i < bones.size(); i++ )
		{
			DirectX::XMMATRIX bone_transform = BoneTransform::ToMatrix( model_pose.bones[bones[i].index].transform );
			out[i] = bones[i].inverse_bind_transform * bone_transform;
		}
	}
}