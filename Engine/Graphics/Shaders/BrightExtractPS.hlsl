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
    float4 col = shaderTexture.Sample(SampleType, input.tex);
    float brightness = dot(col.rgb, float3(0.2126f, 0.7152f, 0.0722f));
    if (brightness > 1.0f)
    {
        return col;
    }
    else
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
}