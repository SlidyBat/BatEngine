cbuffer Matrices : register(b0)
{
    float4x4 world;
    float4x4 viewproj;
};

struct VertexInputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;

    float4x4 wvp = mul(world, viewproj);
    output.position = mul(float4(input.position, 1.0f), wvp);
    output.tex = input.tex;

    return output;
}