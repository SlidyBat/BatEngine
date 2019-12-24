#include "CommonVS.hlsli"

cbuffer Matrices : register(B_SLOT_TRANSFORM)
{
	float4x4 world;
	float4x4 viewproj;
};

struct VertexInputType
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD;
#ifdef HAS_TANGENT
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
#endif
#ifdef HAS_BONES
	uint4  boneids : BONEID;
	float4 boneweights : BONEWEIGHT;
#endif
#ifdef INSTANCED
	float4x4 instance_world : INSTANCE_DATA;
#endif
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 world_pos : POSITION;
	float3 normal : NORMAL;
#ifdef HAS_TANGENT
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
#endif
	float2 tex : TEXCOORD;
};

PixelInputType main(VertexInputType input)
{
	PixelInputType output;
	
	float4 pos = float4(input.position, 1.0f);
	float4 normal = float4(input.normal, 0.0f);
#ifdef HAS_TANGENT
	float4 tangent = float4(input.tangent, 0.0f);
	float4 bitangent = float4(input.bitangent, 0.0f);
#endif
	
	float4x4 final_world = world;

#ifdef HAS_BONES
	float4x4 bone_transform = BoneTransforms[input.boneids.x] * input.boneweights.x;
	bone_transform += BoneTransforms[input.boneids.y] * input.boneweights.y;
	bone_transform += BoneTransforms[input.boneids.z] * input.boneweights.z;
	bone_transform += BoneTransforms[input.boneids.w] * input.boneweights.w;

	final_world = mul(bone_transform, final_world);
#endif
	
#ifdef INSTANCED
	final_world = mul(input.instance_world, final_world);
#endif

	output.world_pos = mul( pos, final_world );
	output.position = mul( output.world_pos, viewproj );

	output.normal = normalize( mul( normal, final_world ) ).xyz;
#ifdef HAS_TANGENT
	output.tangent = normalize( mul( tangent, final_world ) ).xyz;
	output.bitangent = normalize( mul( bitangent, final_world ) ).xyz;
#endif

	output.tex = input.tex;

	return output;
}