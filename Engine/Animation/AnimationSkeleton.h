#pragma once

#include <string>
#include <vector>
#include "Util/MathLib.h"
#include "Core/Entity.h"

namespace Bat
{
	static constexpr size_t MAX_BONES = 256;

	struct BoneData
	{
		std::string name;
		int index;
		Mat3x4 inverse_bind_transform;
	};

	struct BoneTransform
	{
		Vec3 translation = { 0.0f, 0.0f, 0.0f };
		Vec4 rotation = { 0.0f, 0.0f, 0.0f, 0.0f };

		BoneTransform operator+( const BoneTransform& rhs ) const;
		BoneTransform operator*( const BoneTransform& rhs ) const;
		BoneTransform operator*( const float weight ) const;
		static Mat3x4 ToMatrix( const BoneTransform& transform );
		static BoneTransform FromMatrix( const Mat3x4& matrix );
	};

	struct BoneNode
	{
		Entity entity; // The associated entity
		BoneTransform transform;
		int parent_index;
	};

	struct SkeletonPose
	{
		// Vector of every node in the pose (not all are necessarily used in skinning)
		// Ordered in such a way that parent nodes are guaranteed to come before child nodes
		std::vector<BoneNode> bones;
	
		// Converts a given pose from local space to model space
		static SkeletonPose ToModelSpace( SkeletonPose pose );
		// Converts a given model-space pose to a matrix palette for use in skinning
		static void ToMatrixPalette( const SkeletonPose& model_pose, const std::vector<BoneData>& bones, Mat4* out );
		static SkeletonPose Blend( const SkeletonPose* poses, const float* weights, int num_poses, const SkeletonPose& bind_pose );
	};
}