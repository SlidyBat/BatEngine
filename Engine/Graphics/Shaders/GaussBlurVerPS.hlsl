Texture2D shaderTexture;
SamplerState SampleType;

cbuffer Globals
{
    float2 resolution;
    float time;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    const float weight[6] = { 0.0f, 0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f };
    float2 tex_offset = 1.0f / resolution; // gets size of single texel
    float3 result = shaderTexture.Sample(SampleType, input.tex).rgb * weight[0]; // current fragment's contribution
    for (int i = 1; i < 5; ++i)
    {
        result += shaderTexture.Sample(SampleType, input.tex + float2(0.0f, tex_offset.y * i)).rgb * weight[i];
        result += shaderTexture.Sample(SampleType, input.tex - float2(0.0f, tex_offset.y * i)).rgb * weight[i];
    }

    return float4(result, 1.0);
}