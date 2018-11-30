cbuffer Matrices
{
    float4x4 viewproj;
};

struct VertexInputType
{
    float4 position : POSITION;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 tex : TEXCOORD0;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;
    output.position = mul(input.position, viewproj).xyww; // set z = w so that after w divide z = 1.0, skybox will be at the very back
    output.tex = (float3) input.position;

    return output;
}