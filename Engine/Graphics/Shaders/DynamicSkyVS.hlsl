#include "CommonVS.hlsli"

struct VertexInputType
{
	uint id : SV_VertexID;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 clipspace : TEXCOORD0;
};

PixelInputType main(VertexInputType input)
{
	PixelInputType output;
	FullScreenTriangle(input.id, output.position);
	output.position = output.position.xyww; // Set z = w so that after w divide z = 1 (all the way at the back)
	output.clipspace = output.position.xy;
	
	return output;
}