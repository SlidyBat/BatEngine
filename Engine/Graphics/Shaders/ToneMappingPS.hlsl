#include "CommonPS.hlsli"

Texture2D SceneTexture : register(T_SLOT_0);
Texture2D ShadowMap : register(T_SLOT_4);

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

cbuffer ToneMapSettings : register(B_SLOT_0)
{
	float Exposure;
	float3 _pad0;
}

static const float3x3 ACESInputMat =
{
	{ 0.59719, 0.35458, 0.04823 },
	{ 0.07600, 0.90834, 0.01566 },
	{ 0.02840, 0.13383, 0.83777 }
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat =
{
	{ 1.60475, -0.53108, -0.07367 },
	{ -0.10208, 1.10813, -0.00605 },
	{ -0.00327, -0.07276, 1.07602 }
};

float3 RRTAndODTFit(float3 v)
{
	float3 a = v * (v + 0.0245786f) - 0.000090537f;
	float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
	return a / b;
}

float3 ACESFitted(float3 colour)
{
	colour = mul(ACESInputMat, colour);

	// Apply RRT and ODT
	colour = RRTAndODTFit(colour);

	colour = mul(ACESOutputMat, colour);

	// Clamp to [0, 1]
	colour = saturate(colour);

	return colour;
}

float4 main(PixelInputType input) : SV_TARGET
{
	float3 texColour = SceneTexture.Sample(MirrorSampler, input.tex).rgb;
	float3 hdrColour = texColour * Exposure;

	float3 colour = ToSRGBSpace(ACESFitted(hdrColour));
	return float4(colour, 1.0f);
}