#include "CommonPS.hlsli"

Texture2D SceneTexture : register(T_SLOT_0);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

cbuffer ToneMapSettings : register(B_SLOT_0)
{
    float Exposure;
    float A;
    float B;
    float C;
    float D;
    float E;
    float F;
    float W;
}


float3 U2Tonemap(float3 x)
{
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float4 main(PixelInputType input) : SV_TARGET
{
    float3 texColour = SceneTexture.Sample(MirrorSampler, input.tex).rgb;
    float3 hdrColour = texColour * Exposure;

    float ExposureBias = 2.0f;
    float3 curr = U2Tonemap(ExposureBias * hdrColour);

    float3 whiteScale = 1.0f / U2Tonemap(W);
    float3 colour = curr * whiteScale;

    float3 result = ToSRGBSpace(colour);
    return float4(result, 1.0f);
}