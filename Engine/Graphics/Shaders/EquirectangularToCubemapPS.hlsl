#include "CommonPS.hlsli"

Texture2D EquirectangularTexture : register(T_SLOT_0);

struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	return ToSRGBSpace(SampleSphericalMap(EquirectangularTexture, normalize(input.tex)));
}