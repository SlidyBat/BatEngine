#include "CommonVS.hlsli"

cbuffer Matrices : register(B_SLOT_TRANSFORM)
{
	float4x4 world;
	float4x4 viewproj;
};

struct VertexInputType
{
	float3 position : POSITION;
#ifdef HAS_BONES
	uint4  boneids : BONEID;
	float4 boneweights : BONEWEIGHT;
#endif
};

struct PixelInputType
{
	float4 position : SV_POSITION;
};

PixelInputType main( VertexInputType input )
{
	PixelInputType output;
	
	float4x4 final_world = world;

#ifdef HAS_BONES
	float4x4 bone_transform = BoneTransforms[input.boneids.x] * input.boneweights.x;
	bone_transform += BoneTransforms[input.boneids.y] * input.boneweights.y;
	bone_transform += BoneTransforms[input.boneids.z] * input.boneweights.z;
	bone_transform += BoneTransforms[input.boneids.w] * input.boneweights.w;

	final_world = mul(bone_transform, final_world);
#endif

	float3 pos_ws = mul( float4( input.position, 1.0f ), final_world ).xyz;
	output.position = mul( float4( pos_ws, 1.0f ), viewproj );
	
	return output;
}