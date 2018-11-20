cbuffer Matrices
{
    float4x4 world;
    float4x4 viewproj;
};

struct VertexInputType
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 world_pos : POSITION;
    float4 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;
    input.normal.w = 0.0f;

    float4x4 wvp = mul(world, viewproj);
    output.position = mul(input.position, wvp);
    output.world_pos = mul(input.position, world);
    output.normal = normalize(mul(input.normal, world));
    output.tex = input.tex;

    return output;
}