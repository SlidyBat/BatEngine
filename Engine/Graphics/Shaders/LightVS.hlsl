cbuffer Matrices
{
    float4x4 world;
    float4x4 viewproj;
};

struct VertexInputType
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 world_pos : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;

    float4 pos = float4(input.position, 1.0f);

    float4x4 wvp = mul(world, viewproj);
    output.position = mul(pos, wvp);
    output.world_pos = mul(pos, world);
    output.normal = normalize(mul(input.normal, (float3x3) world));
    output.tex = input.tex;

    return output;
}