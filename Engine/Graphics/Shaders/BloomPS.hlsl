#include "Common.hlsli"

Texture2D sceneTexture : register(t0);
Texture2D bloomBlur : register(t1);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    const float gamma = 2.2f;
    const float exposure = 1.0f;
    float3 hdrColour = sceneTexture.Sample(ClampSampler, input.tex).rgb;
    float3 bloomColour = bloomBlur.Sample(ClampSampler, input.tex).rgb;
    hdrColour += bloomColour; // additive blending
    return float4(hdrColour, 1.0);
}