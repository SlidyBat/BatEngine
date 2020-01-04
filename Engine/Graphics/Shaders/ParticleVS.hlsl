#include "CommonVS.hlsli"

cbuffer Matrices : register(B_SLOT_TRANSFORM)
{
	// World not needed since the particle positions are already given in world space
	float4x4 view;
	float4x4 proj;
};

struct VertexInputType
{
	uint index : SV_VertexID;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 colour : COLOR;
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
	
	float t = particle.age / Lifetime;
	float3 quad_pos = quad[vertex_index];
	
	float2 uv = quad_pos.xy * float2(0.5f, -0.5f) + 0.5f;
	float scale = lerp(StartScale, EndScale, t);
	float rotation = particle.rot_velocity * t;
	float3 velocity = mul(particle.velocity, (float3x3)view);
	
	float2x2 rot = float2x2(
		cos(rotation), -sin(rotation),
		sin(rotation), cos(rotation) );
	quad_pos.xy = mul(quad_pos.xy, rot);
	quad_pos *= scale;
	quad_pos += dot(quad_pos, velocity) * velocity * MotionBlur;
	
	output.position = float4(particle.position, 1.0f);
	output.position = mul(output.position, view);
	output.position.xyz += quad_pos;
	output.position = mul(output.position, proj);
	
	output.tex = uv;

	output.colour = particle.colour;
	output.colour.a *= lerp(StartAlpha, EndAlpha, t);
	
	return output;
}