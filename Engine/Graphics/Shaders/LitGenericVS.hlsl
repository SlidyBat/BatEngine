cbuffer Matrices : register(b0)
{
    float4x4 world;
    float4x4 viewproj;
};

struct VertexInputType
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
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

    float4x4 wvp = mul(world, viewproj);

    float4 pos = float4(input.position, 1.0f);
    output.position = mul(pos, wvp);
    output.world_pos = mul(pos, world);

    float3x3 world_trunc = (float3x3) world;
    output.normal = normalize( mul( input.normal, world_trunc ) );
    output.tangent = normalize( mul( input.tangent, world_trunc ) );
    output.bitangent = normalize( mul( input.bitangent, world_trunc ) );
    output.tex = input.tex;

    return output;
}