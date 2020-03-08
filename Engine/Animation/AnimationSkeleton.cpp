#include "PCH.h"
#include "AnimationSkeleton.h"

namespace Bat
{
	BoneTransform BoneTransform::operator+( const BoneTransform& rhs ) const
	{
		BoneTransform added;
		added.translation = translation + rhs.translation;
		float dot = Vec4::Dot( rotation, rhs.rotation );
		float sign = (dot > 0.0f) ? 1.0f : -1.0f;
		added.rotation = rotation + rhs.rotation * sign;
		return added;
	}
	BoneTransform BoneTransform::operator*( const BoneTransform& rhs ) const
	{
		// Wildly inefficient but trying to concatenate translation/rotation myself wasn't working out
		return FromMatrix( ToMatrix( *this ) * ToMatrix( rhs ) );
	}
	BoneTransform BoneTransform::operator*( float weight ) const
	{
		BoneTransform weighted;
		weighted.translation = translation * weight;
		weighted.rotation = rotation * weight;
		return weighted;
	}
	Mat4 BoneTransform::ToMatrix( const BoneTransform& transform )
	{
		return Mat4::RotateQuat( transform.rotation ) * Mat4::Translate( transform.translation );
	}
	BoneTransform BoneTransform::FromMatrix( const Mat4& matrix )
	{
		BoneTransform transform;
		transform.translation = matrix.GetTranslation();
		transform.rotation = matrix.GetRotationQuat();

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
	void SkeletonPose::ToMatrixPalette( const SkeletonPose& model_pose, const std::vector<BoneData>& bones, Mat4* out )
	{
		for( size_t i = 0; i < bones.size(); i++ )
		{
			Mat4 bone_transform = BoneTransform::ToMatrix( model_pose.bones[bones[i].index].transform );
			out[i] = bones[i].inverse_bind_transform * bone_transform;
		}
	}
	SkeletonPose SkeletonPose::Blend( const SkeletonPose* poses, const float* weights, int num_poses, const SkeletonPose& bind_pose )
	{
		ASSERT( num_poses >= 1, "Can't blend 0 poses" );

		const size_t num_bones = poses[0].bones.size();
		ASSERT( std::all_of( poses, poses + num_poses, [=]( const SkeletonPose& pose ) {
			return pose.bones.size() == num_bones;
		} ), "Poses size mismatch" );

		SkeletonPose blended;
		blended.bones.reserve( num_bones );

		float accumulated_weight = 0.0f;
		float weight_factor = 1.0f;
		for( int i = 0; i < num_poses; i++ )
		{
			accumulated_weight += weights[i];
		}
		if( accumulated_weight > 1.0f )
		{
			weight_factor = 1.0f / accumulated_weight;
		}

		for( const BoneNode& bone : poses[0].bones )
		{
			BoneNode weighted_bone;
			weighted_bone.parent_index = bone.parent_index;
			weighted_bone.transform = bone.transform * weights[0] * weight_factor;
			weighted_bone.entity = bone.entity;
			blended.bones.push_back( weighted_bone );
		}

		for( int pose_idx = 1; pose_idx < num_poses; pose_idx++ )
		{
			if( Math::CloseEnough( weights[pose_idx], 0.0f ) )
			{
				continue;
			}

			const SkeletonPose& pose = poses[pose_idx];
			const float weight = weights[pose_idx];

			for( size_t bone_idx = 0; bone_idx < num_bones; bone_idx++ )
			{
				BoneNode& blended_bone = blended.bones[bone_idx];
				const BoneNode& bone = pose.bones[bone_idx];

				blended_bone.transform = blended_bone.transform + (bone.transform * weight * weight_factor);
			}
		}

		if( accumulated_weight < 1.0f )
		{
			float bind_weight = 1.0f - accumulated_weight;

			for( size_t bone_idx = 0; bone_idx < num_bones; bone_idx++ )
			{
				BoneNode& blended_bone = blended.bones[bone_idx];
				const BoneNode& bone = bind_pose.bones[bone_idx];

				blended_bone.transform = blended_bone.transform + (bone.transform * bind_weight);
			}
		}

		for( BoneNode& bone : blended.bones )
		{
			bone.transform.rotation.Normalize();
		}

		return blended;
	}
}