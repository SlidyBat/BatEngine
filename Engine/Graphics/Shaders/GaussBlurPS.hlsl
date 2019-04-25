#include "CommonPS.hlsli"

Texture2D SceneTexture : register(T_SLOT_0);

cbuffer Settings : register(B_SLOT_0)
{
    bool Horizontal;
}

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    const float offsets[] = { 0.0f, 1.3846153846f, 3.2307692308f };
    const float weight[] = { 0.2270270270f, 0.3162162162f, 0.0702702703f };
    
    float4 result = SceneTexture.Sample(ClampSampler, input.tex) * weight[0]; // current fragment's contribution
    if( Horizontal )
    {
        for (int i = 1; i < 3; ++i)
        {
            result += SceneTexture.Sample( ClampSampler, input.tex + float2(offsets[i], 0.0f) / Globals.Resolution.x ) * weight[i];
            result += SceneTexture.Sample( ClampSampler, input.tex - float2(offsets[i], 0.0f) / Globals.Resolution.x ) * weight[i];
        }
    }
    else
    {
        for( int i = 1; i < 3; ++i )
        {
            result += SceneTexture.Sample( ClampSampler, input.tex + float2(0.0f, offsets[i]) / Globals.Resolution.y ) * weight[i];
            result += SceneTexture.Sample( ClampSampler, input.tex - float2(0.0f, offsets[i]) / Globals.Resolution.y ) * weight[i];
        }
    }

    return float4(result.rgb, 1.0);
}