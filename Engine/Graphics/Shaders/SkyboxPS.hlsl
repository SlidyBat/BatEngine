#include "CommonPS.hlsli"

TextureCube SkyboxTexture : register(T_SLOT_0);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    return ToLinearSpace(SkyboxTexture.SampleLevel(WrapSampler, input.tex, 5.0f));
}