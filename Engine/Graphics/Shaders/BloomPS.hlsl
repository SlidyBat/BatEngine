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
    float3 texColour = SceneTexture.Sample(MirrorSampler, input.tex).rgb;
    float3 bloomColour = BlurTexture.Sample(MirrorSampler, input.tex).rgb;
    return float4(texColour + bloomColour, 1.0f);
}