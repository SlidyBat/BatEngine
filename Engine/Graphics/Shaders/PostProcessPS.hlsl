Texture2D shaderTexture;
SamplerState SampleType;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 initial = shaderTexture.Sample(SampleType, input.tex);
    float average = 0.2126 * initial.r + 0.7152 * initial.g + 0.0722 * initial.b;
    return float4(average, average, average, 1.0f);
}