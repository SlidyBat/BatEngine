#include "CommonVS.hlsli"

cbuffer Matrices : register(B_SLOT_TRANSFORM)
{
	// World not needed since the particle positions are already given in world space
	float4x4 view;
	float4x4 proj;
};

#define MAX_PARTICLES 1000

struct Particle
{
	float3 position;
	float _pad0;
};

cbuffer ParticlesBuf : register(B_SLOT_PARTICLES)
{
	Particle Particles[MAX_PARTICLES];
}

struct VertexInputType
{
	uint index : SV_VertexID;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

static const float3 quad[] =
{
	float3(-1.0f, -1.0f, 0.0f), // 0
	float3( 1.0f, -1.0f, 0.0f), // 1
	float3(-1.0f,  1.0f, 0.0f), // 2
	float3(-1.0f,  1.0f, 0.0f), // 3
	float3( 1.0f, -1.0f, 0.0f), // 4
	float3( 1.0f,  1.0f, 0.0f), // 5
};

PixelInputType main(VertexInputType input)
{
	PixelInputType output;

	uint vertex_index = input.index % 6;
	uint particle_index = input.index / 6;
	Particle particle = Particles[particle_index];
	
	float3 quad_pos = quad[vertex_index];
	float2 uv = quad_pos.xy * float2(0.5f, -0.5f) + 0.5f;
	
	output.position = float4(particle.position, 1.0f);
	output.position = mul(output.position, view);
	output.position.xyz += quad_pos * 0.2f;
	output.position = mul(output.position, proj);
	
	output.tex = uv;

	return output;
}