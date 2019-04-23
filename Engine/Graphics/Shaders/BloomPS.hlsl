#include "CommonPS.hlsli"

Texture2D SceneTexture : register(T_SLOT_0);
Texture2D BlurTexture : register(T_SLOT_1);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    const float gamma = 2.2f;
    const float exposure = 1.0f;
    float3 hdrColour = SceneTexture.Sample(ClampSampler, input.tex).rgb;
    float3 bloomColour = BlurTexture.Sample(ClampSampler, input.tex).rgb;
    hdrColour += bloomColour; // additive blending
    return float4(hdrColour, 1.0);
}