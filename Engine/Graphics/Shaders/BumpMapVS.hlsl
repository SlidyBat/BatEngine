cbuffer Matrices
{
    float4x4 world;
    float4x4 viewproj;
};

struct VertexInputType
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float2 tex : TEXCOORD;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 world_pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 tex : TEXCOORD;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;
    input.normal.w = 0.0f;

    float4x4 wvp = mul(world, viewproj);
    output.position = mul(input.position, wvp);
    output.world_pos = mul(input.position, world);
    output.normal = normalize(mul(input.normal, world).xyz);
    output.tangent = normalize(mul(input.tangent, world).xyz);
    output.bitangent = normalize(mul(input.bitangent, world).xyz);
    if (dot(cross(output.normal, output.tangent), output.bitangent) < 0.0)
        output.tangent = -output.tangent;
    output.tex = input.tex;

    return output;
}