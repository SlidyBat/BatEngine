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
#define S_SLOT_LINEAR_WRAP s6
#define S_SLOT_LINEAR_CLAMP s7
#define S_SLOT_0 s8
#define S_SLOT_1 s9
#define S_SLOT_2 s10
#define S_SLOT_3 s11
#define S_SLOT_4 s12
#define S_SLOT_5 s13

// Global samplers
SamplerState WrapSampler : register(S_SLOT_WRAP);
SamplerState ClampSampler : register(S_SLOT_CLAMP);
SamplerState MirrorSampler : register(S_SLOT_MIRROR);
SamplerState BorderSampler : register(S_SLOT_BORDER);
SamplerComparisonState CompareDepthSampler : register(S_SLOT_CMP_DEPTH);
SamplerState PointSampler : register(S_SLOT_POINT);
SamplerState LinearWrapSampler : register(S_SLOT_LINEAR_WRAP);
SamplerState LinearClampSampler : register(S_SLOT_LINEAR_CLAMP);

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
	return tex.Sample(LinearWrapSampler, uv);
}

float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
	return float2((float) i / (float) N, RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
	float a = roughness * roughness;
	float phi = 2.0f * PI * Xi.x;
	float cos_theta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
	float sin_theta = sqrt(1.0f - cos_theta * cos_theta);

	float3 H;
	H.x = cos(phi) * sin_theta;
	H.y = sin(phi) * sin_theta;
	H.z = cos_theta;

	float3 up = abs(N.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
	float3 tangent = normalize(cross(up, N));
	float3 bitangent = cross(N, tangent);

	float3 dir = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(dir);
}