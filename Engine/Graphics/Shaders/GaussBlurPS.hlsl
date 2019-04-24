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
    const float weight[] = { 0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f };
    float2 tex_offset = 1.0f / Globals.Resolution; // gets size of single texel
    float4 result = SceneTexture.Sample(ClampSampler, input.tex) * weight[0]; // current fragment's contribution
    
    if( Horizontal )
    {
        for (int i = 1; i < 5; ++i)
        {
            result += SceneTexture.Sample(ClampSampler, input.tex + float2(tex_offset.x * i, 0.0f)) * weight[i];
            result += SceneTexture.Sample(ClampSampler, input.tex - float2(tex_offset.x * i, 0.0f)) * weight[i];
        }
    }
    else
    {
        for( int i = 1; i < 5; ++i )
        {
            result += SceneTexture.Sample( ClampSampler, input.tex + float2(0.0f, tex_offset.y * i) ) * weight[i];
            result += SceneTexture.Sample( ClampSampler, input.tex - float2(0.0f, tex_offset.y * i) ) * weight[i];
        }
    }

    return float4(result.rgb, 1.0);
}