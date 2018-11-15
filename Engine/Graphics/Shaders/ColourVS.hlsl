cbuffer Matrices
{
    float4x4 world;
    float4x4 viewproj;
};

struct VertexInputType
{
    float4 position : POSITION;
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
    output.position = mul(input.position, wvp);
    output.colour = input.colour;

    return output;
}