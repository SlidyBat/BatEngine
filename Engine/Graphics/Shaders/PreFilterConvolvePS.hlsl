#include "CommonPS.hlsli"

TextureCube EnvironmentMap : register(T_SLOT_0);

cbuffer PrefilterConvolveInfo : register(B_SLOT_0)
{
	float Roughness;
	float3 _pad0;
}

struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 tex : TEXCOORD0;
};

static const uint SAMPLE_COUNT = 1024;

float4 main(PixelInputType input) : SV_TARGET
{
	float3 N = normalize(input.tex);
	float3 R = N;
	float3 V = R;
	
	
	float delta = 0.025f;
	int num_samples = 0.0f;
	
	float total_weight = 0.0f;
	float3 prefiltered = float3(0.0f, 0.0f, 0.0f);
	for (uint i = 0; i < SAMPLE_COUNT; ++i)
	{
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 H = ImportanceSampleGGX(Xi, N, Roughness);
		float3 L = normalize(2.0f * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0f);
		if (NdotL > 0.0f)
		{
			prefiltered += ToLinearSpace(EnvironmentMap.Sample(LinearWrapSampler, L).rgb) * NdotL;
			total_weight += NdotL;
		}
	}
	prefiltered /= total_weight;

	return float4(prefiltered, 1.0f);
}