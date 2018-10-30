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

    output.position = input.position;
    output.position.w = 1.0f;
    output.colour = input.colour;

    return output;
}