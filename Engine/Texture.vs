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

PixelInputType VSMain( VertexInputType input )
{
	PixelInputType output;

	output.position = input.position;
	output.position.w = 1.0f;
	output.tex = input.tex;

	return output;
}