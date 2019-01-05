#include "Common.hlsli"

Texture2D shaderTexture;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    return shaderTexture.Sample(WrapSampler, input.tex);;
}