#include "CommonPS.hlsli"

Texture2D SceneTexture : register(T_SLOT_0);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main( PixelInputType input ) : SV_TARGET
{
	const float weight[6] = { 0.0f, 0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f };
    float2 tex_offset = 1.0f / Globals.Resolution.x; // gets size of single texel
	float3 result = SceneTexture.Sample( ClampSampler, input.tex ).rgb * weight[0]; // current fragment's contribution
	for( int i = 1; i < 5; ++i )
    {
		result += SceneTexture.Sample( ClampSampler, input.tex + float2(0.0f, tex_offset.y * i) ).rgb * weight[i];
		result += SceneTexture.Sample( ClampSampler, input.tex - float2(0.0f, tex_offset.y * i) ).rgb * weight[i];
    }

    return float4(result, 1.0);
}