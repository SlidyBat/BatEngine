#pragma once

#include <string>
#include <vector>
#include "MathLib.h"

namespace Bat
{
	static constexpr size_t MAX_BONES = 256;

	struct BoneData
	{
		std::string name;
		int index;
		DirectX::XMMATRIX inverse_bind_transform;
	};

	struct BoneTransform
	{
		Vec3 translation = { 0.0f, 0.0f, 0.0f };
		Vec4 rotation = { 0.0f, 0.0f, 0.0f, 0.0f };

		BoneTransform operator*( const BoneTransform& rhs );
		static DirectX::XMMATRIX ToMatrix( const BoneTransform& transform );
		static BoneTransform FromMatrix( DirectX::XMMATRIX matrix );
	};

	struct BoneNode
	{
		BoneTransform transform;
		int parent_index;
	};

	struct SkeletonPose
	{
		std::vector<BoneNode> bones;
	
		// Converts a given pose from local space to model space
		static SkeletonPose ToModelSpace( SkeletonPose pose );
		// Converts a given model-space pose to a matrix palette for use in skinning
		static void ToMatrixPalette( const SkeletonPose& model_pose, const std::vector<BoneData>& bones, DirectX::XMMATRIX* out );
	};
}