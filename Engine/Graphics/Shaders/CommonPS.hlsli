struct Material
{
	float4  GlobalAmbient;

	float4  AmbientColor;

	float4  EmissiveColor;

	float4  DiffuseColor;

	float4  SpecularColor;

	float4  Reflectance;

	float   Opacity;
	float   SpecularPower;
	float   IndexOfRefraction;
	bool    HasAmbientTexture;

	bool    HasEmissiveTexture;
	bool    HasDiffuseTexture;
	bool    HasSpecularTexture;
	bool    HasSpecularPowerTexture;

	bool    HasNormalTexture;
	bool    HasBumpTexture;
	bool    HasOpacityTexture;
	float   BumpIntensity;

	float   SpecularScale;
	float   AlphaThreshold;
	float2  Padding;
};

#define LIGHT_POINT       0
#define LIGHT_DIRECTIONAL 1
#define LIGHT_SPOT        2

#define MAX_LIGHTS 16
struct Light
{
	float3 Position;
	float _pad0;

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
};

// Defines for cbuffer slots
// Use these instead of normal slots to ensure that you don't overwrite slots for global buffers
#define B_SLOT_GLOBALS   b0
#define B_SLOT_PARTICLES b1
#define B_SLOT_0 b2
#define B_SLOT_1 b3
#define B_SLOT_2 b4
#define B_SLOT_3 b5
#define B_SLOT_4 b6
#define B_SLOT_5 b7
#define B_SLOT_6 b8

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
#define S_SLOT_0 s4
#define S_SLOT_1 s5
#define S_SLOT_2 s6
#define S_SLOT_3 s7
#define S_SLOT_4 s8
#define S_SLOT_5 s9
#define S_SLOT_6 s10
#define S_SLOT_7 s11

// Global samplers
SamplerState WrapSampler : register(S_SLOT_WRAP);
SamplerState ClampSampler : register(S_SLOT_CLAMP);
SamplerState MirrorSampler : register(S_SLOT_MIRROR);
SamplerState BorderSampler : register(S_SLOT_BORDER);

// Defines for texture slots
// Use these instead of normal slots to ensure that you don't overwrite slots for global textures
#define T_SLOT_0 t0
#define T_SLOT_1 t1
#define T_SLOT_2 t2
#define T_SLOT_3 t3
#define T_SLOT_4 t4
#define T_SLOT_5 t5
#define T_SLOT_6 t6
#define T_SLOT_7 t7

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