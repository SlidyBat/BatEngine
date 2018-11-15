cbuffer Matrices
{
    float4x4 world;
    float4x4 viewproj;
};

struct VertexInputType
{
    float4 position : POSITION;
    float3 normal : NORMAL;
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
    output.position = mul(input.position, wvp);
    output.tex = input.tex;

    return output;
}