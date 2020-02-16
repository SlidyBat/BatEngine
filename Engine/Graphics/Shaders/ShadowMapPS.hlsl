#include "CommonPS.hlsli"

cbuffer Material : register(B_SLOT_0)
{
	float4 BaseColourFactor;
	bool HasBaseColourTexture;
	float AlphaCutoff;
	float2 _pad0;
}

Texture2D BaseColourTexture : register(T_SLOT_0);

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float4 colour = BaseColourFactor;
	if (HasBaseColourTexture)
	{
		colour *= BaseColourTexture.Sample(WrapSampler, input.tex);
	}
	
	clip(colour - AlphaCutoff);
	return colour;
}