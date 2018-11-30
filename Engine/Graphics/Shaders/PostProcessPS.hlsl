Texture2D shaderTexture;
SamplerState SampleType;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    const float offset = 1.0 / 300.0;

    const float2 offsets[9] =
    {
        float2(-offset, offset), // top-left
        float2(0.0f, offset), // top-center
        float2(offset, offset), // top-right
        float2(-offset, 0.0f), // center-left
        float2(0.0f, 0.0f), // center-center
        float2(offset, 0.0f), // center-right
        float2(-offset, -offset), // bottom-left
        float2(0.0f, -offset), // bottom-center
        float2(offset, -offset) // bottom-right    
    };

    float kernel[9] =
    {
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    };

    float3 colour = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 9; i++)
    {
        float3 sampletex = shaderTexture.Sample(SampleType, input.tex + offsets[i]).rgb;
        colour += sampletex * kernel[i];
    }

    return float4(colour, 1.0f);
}