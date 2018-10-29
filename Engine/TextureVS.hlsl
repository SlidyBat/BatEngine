cbuffer Matrices
{
    float4x4 mat;
};

struct VertexInputType
{
    float4 position : POSITION;
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

    output.position = mul(input.position, mat);
    output.tex = input.tex;

    return output;
}