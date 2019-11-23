#include "CommonVS.hlsli"

cbuffer Matrices : register(B_SLOT_TRANSFORM)
{
    float4x4 world;
    float4x4 viewproj;
};

struct VertexInputType
{
    float3 position : POSITION;
    float4 colour : COLOR;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 colour : COLOR;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;

    float4x4 wvp = mul(world, viewproj);
    output.position = mul(float4(input.position, 1.0f), wvp);
    output.colour = input.colour;

    return output;
}