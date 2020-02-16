#include "CommonPS.hlsli"

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

static const uint SAMPLE_COUNT = 1024;

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float a = roughness;
	float k = (a * a) / 2.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

float2 IntegrateBrdf(float NdotV, float roughness)
{
	float3 V;
	V.x = sqrt(1.0f - NdotV*NdotV);
	V.y = 0.0f;
	V.z = NdotV;
	
	float A = 0.0f;
	float B = 0.0f;
	
	float3 N = float3(0.0f, 0.0f, 1.0f);

	for (uint i = 0; i < SAMPLE_COUNT; i++)
	{
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 H = ImportanceSampleGGX(Xi, N, roughness);
		float3 L = normalize(2.0f * dot(V, H) * H - V);

		float NdotL = max(L.z, 0.0f);
		float NdotH = max(H.z, 0.0f);
		float VdotH = max(dot(V, H), 0.0f);

		if (NdotL > 0.0f)
		{
			float G = GeometrySmith(N, V, L, roughness);
			float G_Vis = (G * VdotH) / (NdotH * NdotV);
			float Fc = pow(1.0f - VdotH, 5.0f);

			A += (1.0f - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	A /= SAMPLE_COUNT;
	B /= SAMPLE_COUNT;
	
	return float2(A, B);
}

float2 main(PixelInputType input) : SV_TARGET
{
	return IntegrateBrdf(input.tex.x, input.tex.y);
}