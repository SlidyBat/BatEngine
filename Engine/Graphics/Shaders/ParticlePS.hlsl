#include "CommonPS.hlsli"

Texture2D SceneTexture : register(T_SLOT_0);

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float alpha = SceneTexture.Sample(WrapSampler, input.tex).r;
	return float4(1.0f, 1.0f, 1.0f, alpha);
}