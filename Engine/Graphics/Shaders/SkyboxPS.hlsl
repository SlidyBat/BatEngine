TextureCube cubemap;
SamplerState SampleType;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    return cubemap.Sample(SampleType, input.tex);
}