#include "CommonPS.hlsli"

Texture2D SceneTexture : register(T_SLOT_0);

cbuffer ThresholdBuf : register(B_SLOT_0)
{
	float BrightThreshold;
}

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 col = SceneTexture.Sample(ClampSampler, input.tex);
    float brightness = dot(col.rgb, float3(0.2126f, 0.7152f, 0.0722f));
    if (brightness > BrightThreshold)
    {
        return col;
    }
    else
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
}