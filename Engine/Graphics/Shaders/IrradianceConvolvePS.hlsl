#include "CommonPS.hlsli"

TextureCube EnvironmentMap : register(T_SLOT_0);

struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float3 normal = normalize(input.tex);
	float3 irradiance = float3(0.0f, 0.0f, 0.0f);
	
	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 right = cross(up, normal);
	up = cross(normal, right);
	
	float delta = 0.025f;
	int num_samples = 0.0f;
	
	for (float phi = 0.0f; phi < 2.0f * PI; phi += delta)
	{
		for (float theta = 0.0f; theta < 0.5f * PI; theta += delta)
		{
			float3 tangent = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			float3 dir = tangent.x * right + tangent.y * up + tangent.z * normal;
			
			irradiance += EnvironmentMap.Sample(LinearSampler, dir).rgb * cos(theta) * sin(theta);
			num_samples++;
		}
	}
	irradiance *= PI * (1.0f / num_samples);
	
	return float4(irradiance, 1.0f);
}