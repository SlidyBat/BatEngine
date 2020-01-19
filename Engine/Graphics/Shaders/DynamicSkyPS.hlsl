#include "CommonPS.hlsli"

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 clipspace : TEXCOORD0;
};

// Atmosphere based on: https://www.shadertoy.com/view/Ml2cWG
// Cloud noise based on: https://www.shadertoy.com/view/4tdSWr

// Returns distance from ray origin to plane if hit, otherwise 0
float TracePlane(float3 origin, float3 dir, float3 plane_origin, float3 plane_normal)
{
	return dot(plane_normal, (plane_origin - origin) / dot(plane_normal, dir));
}

float2 hash(float2 p)
{
	p = float2(dot(p, float2(127.1, 311.7)), dot(p, float2(269.5, 183.3)));
	return -1.0 + 2.0 * frac(sin(p) * 43758.5453123);
}
float noise(in float2 p)
{
	const float K1 = 0.366025404; // (sqrt(3)-1)/2;
	const float K2 = 0.211324865; // (3-sqrt(3))/6;
	float2 i = floor(p + (p.x + p.y) * K1);
	float2 a = p - i + (i.x + i.y) * K2;
	float2 o = (a.x > a.y) ? float2(1.0, 0.0) : float2(0.0, 1.0); //float2 of = 0.5 + 0.5*float2(sign(a.x-a.y), sign(a.y-a.x));
	float2 b = a - o + K2;
	float2 c = a - 1.0 + 2.0 * K2;
	float3 h = max(0.5 - float3(dot(a, a), dot(b, b), dot(c, c)), 0.0);
	float3 n = h * h * h * h * float3(dot(a, hash(i + 0.0)), dot(b, hash(i + o)), dot(c, hash(i + 1.0)));
	return dot(n, float3(70.0, 70.0, 70.0));
}

// Returns sky color modulated by the sun and clouds
//	V	: view direction
float3 GetDynamicSkyColor(in float3 V, bool sun_enabled = true, bool clouds_enabled = true, bool dark_enabled = false)
{
	float t = cos(Globals.Time) * 0.5f + 0.5f;
	const float cloudiness = 0.2f;
	const float cloudScale = 0.0003f;
	const float cloudSpeed = 0.1f;
	const float3 horizonColor = lerp(float3(0.2f, 0.05f, 0.15f), float3(0.3f, 0.3f, 0.4f), t);
	const float3 skyColor = lerp(float3(0.4f, 0.05f, 0.1f), float3(37.0f / 255.0f, 61.0f / 255.0f, 142.0f / 255.0f), t);
	const float3 sunDirection = normalize(float3(1.0f, t, 1.0f));
	const float3 sunColor = float3(1.0f, 1.0f, 1.0f);
	const float fog = (1 - t);
	sun_enabled = sun_enabled && any(sunColor);

	const float zenith = V.y; // how much is above (0: horizon, 1: directly above)
	const float sunScatter = saturate(sunDirection.y + 0.1f); // how much the sun is directly above. Even if sunis at horizon, we add a constant scattering amount so that light still scatters at horizon

	const float atmosphereDensity = 0.5 + fog; // constant of air density, or "fog height" as interpreted here (bigger is more obstruction of sun)
	const float zenithDensity = atmosphereDensity / pow(max(0.000001f, zenith), 0.75f);
	const float sunScatterDensity = atmosphereDensity / pow(max(0.000001f, sunScatter), 0.75f);

	const float3 aberration = float3(0.39, 0.57, 1.0); // the chromatic aberration effect on the horizon-zenith fade line
	const float3 skyAbsorption = saturate(exp2(aberration * -zenithDensity) * 2.0f); // gradient on horizon
	const float3 sunAbsorption = sun_enabled ? saturate(sunColor * exp2(aberration * -sunScatterDensity) * 2.0f) : 1; // gradient of sun when it's getting below horizon

	const float sunAmount = distance(V, sunDirection); // sun falloff descreasing from mid point
	const float rayleigh = sun_enabled ? 1.0 + pow(1.0 - saturate(sunAmount), 2.0) * PI * 0.5 : 1;
	const float mie_disk = saturate(1.0 - pow(sunAmount, 0.1));
	const float3 mie = mie_disk * mie_disk * (3.0 - 2.0 * mie_disk) * 2.0 * PI * sunAbsorption;

	const float3 sun = smoothstep(0.03, 0.026, sunAmount) * sunColor * 50.0 * skyAbsorption; // sun disc

	float3 sky = lerp(horizonColor, skyColor * zenithDensity * rayleigh, skyAbsorption);
	sky = lerp(sky * skyAbsorption, sky, sunScatter); // when sun goes below horizon, absorb sky color
	if (sun_enabled)
	{
		sky += sun;
		sky += mie;
	}
	sky *= (sunAbsorption + length(sunAbsorption)) * 0.5f; // when sun goes below horizon, fade out whole sky
	sky *= 0.25; // exposure level

	if (dark_enabled)
	{
		sky = max(pow(saturate(dot(sunDirection, V)), 64) * sunColor, 0) * skyAbsorption;
	}

	if (clouds_enabled)
	{
		if (cloudiness <= 0)
		{
			return sky;
		}

		// Trace a cloud layer plane:
		const float3 o = Globals.CameraPos;
		const float3 d = V;
		const float3 planeOrigin = float3(0, 1000, 0);
		const float3 planeNormal = float3(0, -1, 0);
		const float t = TracePlane(o, d, planeOrigin, planeNormal);

		if (t < 0)
		{
			return sky;
		}

		const float3 cloudPos = o + d * t;
		const float2 cloudUV = cloudPos.xz * cloudScale;
		const float cloudTime = Globals.Time * cloudSpeed;
		const float2x2 m = float2x2(1.6, 1.2, -1.2, 1.6);
		const uint quality = 8;

		// rotate uvs like a flow effect:
		float flow = 0;
		{
			float2 uv = cloudUV * 0.5f;
			float amount = 0.1;
			for (uint i = 0; i < quality; i++)
			{
				flow += noise(uv) * amount;
				uv = mul(m, uv);
				amount *= 0.4;
			}
		}


		// Main shape:
		float clouds = 0.0;
		{
			const float time = cloudTime * 0.2f;
			float density = 1.1f;
			float2 uv = cloudUV * 0.8f;
			uv -= flow - time;
			for (uint i = 0; i < quality; i++)
			{
				clouds += density * noise(uv);
				uv = mul(m, uv) + time;
				density *= 0.6f;
			}
		}

		// Detail shape:
		{
			float detail_shape = 0.0;
			const float time = cloudTime * 0.1f;
			float density = 0.8f;
			float2 uv = cloudUV;
			uv -= flow - time;
			for (uint i = 0; i < quality; i++)
			{
				detail_shape += abs(density * noise(uv));
				uv = mul(m, uv) + time;
				density *= 0.7f;
			}
			clouds *= detail_shape + clouds;
			clouds *= detail_shape;
		}


		// lerp between "choppy clouds" and "uniform clouds". Lower cloudiness will produce choppy clouds, but very high cloudiness will switch to overcast unfiform clouds:
		clouds = lerp(clouds * 9.0f * cloudiness + 0.3f, clouds * 0.5f + 0.5f, pow(saturate(cloudiness), 8));
		clouds = saturate(clouds - (1 - cloudiness)); // modulate constant cloudiness
		clouds *= pow(1 - saturate(length(abs(cloudPos.xz * 0.00001f))), 16); //fade close to horizon

		if (dark_enabled)
		{
			sky *= pow(saturate(1 - clouds), 16.0f); // only sun and clouds. Boost clouds to have nicer sun shafts occlusion
		}
		else
		{
			sky = lerp(sky, 1, clouds); // sky and clouds on top
		}
	}

	return sky;
}

float4 main(PixelInputType input) : SV_TARGET
{
	float4 pos_ws = mul(float4(input.clipspace, 0.0f, 1.0f), Globals.CameraInvVP);
	pos_ws.xyz /= pos_ws.w;
	
	float3 dir = normalize(pos_ws.xyz - Globals.CameraPos);
	float3 sky_colour = GetDynamicSkyColor(dir, true, true);
	
	return float4(sky_colour, 1.0f);
}