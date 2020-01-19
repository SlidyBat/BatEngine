#include "CommonVS.hlsli"

struct VertexInputType
{
	uint id : SV_VertexID;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

PixelInputType main(VertexInputType input)
{
	PixelInputType output;
	FullScreenTriangle(input.id, output.position, output.tex);

	return output;
}