#define PI 3.141592654f

struct Material
{
	float4 BaseColourFactor;

	float3 EmissiveFactor;
	float  MetallicFactor;
	
	float  RoughnessFactor;
	bool   HasBaseColourTexture;
	bool   HasMetallicRoughnessTexture;
	bool   HasNormalTexture;
	
	bool   HasOcclusionTexture;
	bool   HasEmissiveTexture;
	float  AlphaCutoff;
	float  _pad0;
};

#define LIGHT_POINT       0
#define LIGHT_DIRECTIONAL 1
#define LIGHT_SPOT        2

#define MAX_SHADOW_SOURCES 16
#define INVALID_SHADOW_MAP_INDEX (~0)
#define NUM_CASCADES 3

#define MAX_LIGHTS 16
struct Light
{
	float3 Position;
	int ShadowIndex;

	float3 Direction;
	float SpotlightAngle;

	float3 Colour;
	float _pad3;

	float Range;
	float Intensity;
	uint Type;
	float Pad;
};

struct GlobalsBuf
{
	// width/height of screen
	float2 Resolution;
	// time in seconds since program was launched
	float Time;
	// time since last frame
	float DeltaTime;
	// camera position in world space
	float3 CameraPos;
	float pad0;
	
	float4x4 CameraInvVP;
};

// Defines for cbuffer slots
// Use these instead of normal slots to ensure that you don't overwrite slots for global buffers
#define B_SLOT_GLOBALS        b0
#define B_SLOT_PARTICLES      b1
#define B_SLOT_SHADOWMATRICES b2
#define B_SLOT_0 b3
#define B_SLOT_1 b4
#define B_SLOT_2 b5
#define B_SLOT_3 b6
#define B_SLOT_4 b7
#define B_SLOT_5 b8
#define B_SLOT_6 b9

// Global buffers
cbuffer GlobalsBuf : register(B_SLOT_GLOBALS)
{
	GlobalsBuf Globals;
};

// Defines for sampler slots
// Use these instead of normal slots to ensure that you don't overwrite slots for global samplers
#define S_SLOT_WRAP   s0
#define S_SLOT_CLAMP  s1
#define S_SLOT_MIRROR s2
#define S_SLOT_BORDER s3
#define S_SLOT_CMP_DEPTH s4
#define S_SLOT_POINT  s5
#define S_SLOT_LINEAR s6
#define S_SLOT_0 s7
#define S_SLOT_1 s8
#define S_SLOT_2 s9
#define S_SLOT_3 s10
#define S_SLOT_4 s11
#define S_SLOT_5 s12
#define S_SLOT_6 s13

// Global samplers
SamplerState WrapSampler : register(S_SLOT_WRAP);
SamplerState ClampSampler : register(S_SLOT_CLAMP);
SamplerState MirrorSampler : register(S_SLOT_MIRROR);
SamplerState BorderSampler : register(S_SLOT_BORDER);
SamplerComparisonState CompareDepthSampler : register(S_SLOT_CMP_DEPTH);
SamplerState PointSampler : register(S_SLOT_POINT);
SamplerState LinearSampler : register(S_SLOT_LINEAR);

// Defines for texture slots
// Use these instead of normal slots to ensure that you don't overwrite slots for global textures
#define T_SLOT_SHADOWMAPS t0
#define T_SLOT_0 t1
#define T_SLOT_1 t2
#define T_SLOT_2 t3
#define T_SLOT_3 t4
#define T_SLOT_4 t5
#define T_SLOT_5 t6
#define T_SLOT_6 t7
#define T_SLOT_7 t8

#define GAMMA 2.2f

float3 ToSRGBSpace(float3 colour)
{
	return pow(colour, 1.0 / GAMMA);
}

float4 ToSRGBSpace(float4 colour)
{
	return float4(ToSRGBSpace(colour.rgb), colour.a);
}

float3 ToLinearSpace(float3 colour)
{
	return pow(colour, GAMMA);
}

float4 ToLinearSpace(float4 colour)
{
	return float4(ToLinearSpace(colour.rgb), colour.a);
}

bool IsSaturated(float a)
{
	return saturate(a) == a;
}

bool IsSaturated(float2 a)
{
	return IsSaturated(a.x) && IsSaturated(a.y);
}

bool IsSaturated(float3 a)
{
	return IsSaturated(a.x) && IsSaturated(a.y) && IsSaturated(a.z);
}

bool IsSaturated(float4 a)
{
	return IsSaturated(a.x) && IsSaturated(a.y) && IsSaturated(a.z) && IsSaturated(a.w);
}

static const float2 inv_atan = float2(0.1591f, -0.3183f);
float4 SampleSphericalMap(Texture2D tex, float3 v)
{
	float2 uv = float2(atan2(v.z, v.x), asin(v.y));
	uv *= inv_atan;
	uv += 0.5;
	return tex.Sample(LinearSampler, uv);
}