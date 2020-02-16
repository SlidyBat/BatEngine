#include "CommonPS.hlsli"

static const float3 GlobalAmbient = float3(0.03f, 0.03f, 0.03f);

struct MaterialInfo
{
	float4 Albedo;
	float Metallic;
	float Roughness;
	float3 F0;
};

Texture2D BaseColourTexture : register(T_SLOT_0);
Texture2D MetallicRoughnessTexture : register(T_SLOT_1);
Texture2D NormalTexture : register(T_SLOT_2);
Texture2D OcclusionTexture : register(T_SLOT_3);
Texture2D EmissiveTexture : register(T_SLOT_4);

TextureCube IrradianceMap : register(T_SLOT_5);

Texture2DArray ShadowMap : register(T_SLOT_SHADOWMAPS);

cbuffer ShadowMatrices : register(B_SLOT_SHADOWMATRICES)
{
	float4x4 ShadowMatrix[MAX_SHADOW_SOURCES];
}

cbuffer Material : register(B_SLOT_0)
{
	Material Mat;
}

cbuffer LightParams : register(B_SLOT_1)
{
	uint NumLights;
	Light Lights[MAX_LIGHTS];
}

struct PixelInput
{
	float4 position : SV_POSITION;
	float4 world_pos : POSITION;
	float3 normal : NORMAL;
#ifdef HAS_TANGENT
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
#endif
	float2 tex : TEXCOORD;
};

float Shadow( Light light, float3 pos_ws )
{
	if( light.ShadowIndex == INVALID_SHADOW_MAP_INDEX )
	{
		return 1.0f;
	}
	
	int shadow_index = light.ShadowIndex;
	if( light.Type == LIGHT_DIRECTIONAL )
	{
		for (int cascade = 0; cascade < NUM_CASCADES; cascade++)
		{
			shadow_index = light.ShadowIndex + cascade;
			
			float4 proj_coords = mul(float4(pos_ws, 1.0f), ShadowMatrix[shadow_index]);
			proj_coords.xyz /= proj_coords.w;
			float3 shadow_uv = proj_coords.xyz * float3(0.5f, -0.5f, 0.5f) + 0.5f;
			
			if (IsSaturated(shadow_uv))
			{
				break;
			}
		}
	}
	
	float4 proj_coords = mul( float4( pos_ws, 1.0f ), ShadowMatrix[shadow_index] );
	proj_coords.xyz /= proj_coords.w;
	
	float bias = 0.0005f;
	float current_depth = proj_coords.z + bias;
	
	float2 shadow_uv = proj_coords.xy * float2( 0.5f, -0.5f ) + float2( 0.5f, 0.5f );
	
	int range = 2;
	float shadow = 0.0f;
	[unroll]
	for (int y = -range; y <= range; y++)
	{
		for (int x = -range; x <= range; x++)
		{
			shadow += ShadowMap.SampleCmpLevelZero(CompareDepthSampler, float3(shadow_uv, shadow_index), current_depth, int2(x, y)).r;
		}
	}
	shadow /= (range * 2 + 1) * (range * 2 + 1);

	return shadow;
}

float3 FresnelSchlick(float3 H, float3 V, float3 F0)
{
	float cos_theta = max(dot(H, V), 0.0f);
	return F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
}

float3 FresnelSchlickRoughness(float3 H, float3 V, float3 F0, float roughness)
{
	float cos_theta = max(dot(H, V), 0.0f);
	return F0 + (max(1.0f - roughness, F0) - F0) * pow(1.0 - cos_theta, 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;
	
	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

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

float3 Radiance(MaterialInfo material, float3 L, float3 P, float3 N)
{
	float3 V = normalize(Globals.CameraPos - P);
	float3 H = normalize(L + V);
	
	float NDF = DistributionGGX(N, H, material.Roughness);
	float G   = GeometrySmith(N, V, L, material.Roughness);
	float3 F  = FresnelSchlick(H, V, material.F0);
	
	float3 kS = F;
	float3 kD = (1.0f - kS) * (1.0f - material.Metallic);
	
	float NdotV = max(dot(N, V), 0.0f);
	float NdotL = max(dot(N, L), 0.0f);
	
	float3 numerator = NDF * G * F;
	float denominator = 4.0f * NdotL * NdotV;
	float3 specular = numerator / max(denominator, 0.001f);
	
	float3 diffuse = kD * material.Albedo.rgb / PI;
	
	return (diffuse + specular) * NdotL;
}

float Attenuation(Light light, float distance)
{
	return pow(saturate(1.0 - pow(distance / light.Range, 4.0f)), 2.0f) / (distance * distance + 1);
}


float3 DoPointLight( Light light, MaterialInfo material, float3 P, float3 N )
{
	float3 Lunnormalized = light.Position - P;
	float distance = length( Lunnormalized );
	float3 L = Lunnormalized / distance;

	float attenuation = Attenuation(light, distance);
	float3 radiance = Radiance(material, L, P, N);

	return radiance * attenuation * light.Intensity * light.Colour;
}

float3 DoSpotLight( Light light, MaterialInfo material, float3 P, float3 N )
{
	float3 L = light.Position - P;
	float light_distance = length( L );
	L /= light_distance;

	float min_theta = cos( light.SpotlightAngle );
	float max_theta = lerp( min_theta, 1, 0.5f );
	float theta = dot( light.Direction.xyz, -L );
	float spot_intensity = smoothstep( min_theta, max_theta, theta );
	
	float attenuation = Attenuation(light, light_distance);
	float3 radiance = Radiance(material, L, P, N);

	return radiance * attenuation * spot_intensity * Shadow(light, P);
}

float3 DoDirectionalLight( Light light, MaterialInfo material, float3 P, float3 N )
{
	float3 L = light.Direction;
	float3 radiance = Radiance(material, L, P, N);
	
	return radiance * light.Intensity * light.Colour * Shadow(light, P);
}

float3 DoLight( Light light, MaterialInfo material, float3 world_pos, float3 normal )
{
	switch( light.Type )
	{
	case LIGHT_POINT:
		return DoPointLight( light, material, world_pos, normal );
	case LIGHT_SPOT:
		return DoSpotLight( light, material, world_pos, normal );
	case LIGHT_DIRECTIONAL:
		return DoDirectionalLight( light, material, world_pos, normal );
	default:
		return float3(0.0f, 0.0f, 0.0f);
	}
}

float4 main( PixelInput input ) : SV_TARGET
{
	MaterialInfo material;

	material.Albedo = Mat.BaseColourFactor;
	if (Mat.HasBaseColourTexture)
	{
		material.Albedo *= ToLinearSpace(BaseColourTexture.Sample(WrapSampler, input.tex));
	}
	
#ifdef MASK_ALPHA
	clip(material.Albedo.a - Mat.AlphaCutoff);
#endif
	
	float3 normal = normalize(input.normal);
#ifdef HAS_TANGENT
	if( Mat.HasNormalTexture )
	{
		float3 normal_sample = NormalTexture.Sample( WrapSampler, input.tex ).xyz;
		normal = normal_sample * 2.0f - 1.0f;
		normal.y = -normal.y;
		
		float3x3 tbn = float3x3(normalize(input.tangent), normalize(input.bitangent), normalize(input.normal));
		normal = normalize( mul( normal, tbn ) );
	}
#endif

	float3 world_pos = input.world_pos.xyz;
	
	material.Metallic = Mat.MetallicFactor;
	material.Roughness = Mat.RoughnessFactor;
	if( Mat.HasMetallicRoughnessTexture )
	{
		float4 mr_colour = MetallicRoughnessTexture.Sample(WrapSampler, input.tex);
		material.Metallic *= mr_colour.b;
		material.Roughness *= mr_colour.g;
	}
	material.Roughness = max(0.025f, material.Roughness);

	const float dielectric_specular = float3(0.04f, 0.04f, 0.04f);
	material.F0 = lerp(dielectric_specular, material.Albedo.rgb, material.Metallic);
	
	float occlusion = 1.0f;
	if( Mat.HasOcclusionTexture )
	{
		occlusion = OcclusionTexture.Sample(WrapSampler, input.tex);
	}

	float3 emissive = Mat.EmissiveFactor;
	if( Mat.HasEmissiveTexture )
	{
		emissive *= ToLinearSpace(EmissiveTexture.Sample(WrapSampler, input.tex));
	}

	float3 colour = 0.0f;
	for( uint i = 0; i < NumLights; i++ )
	{
		colour += DoLight( Lights[i], material, world_pos, normal );
	}
	
	float3 view_dir = normalize(Globals.CameraPos - world_pos);
	float3 kS = FresnelSchlickRoughness(normal, view_dir, material.F0, material.Roughness);
	float3 kD = (1.0f - kS) * (1.0f - material.Metallic);
	float3 irradiance = IrradianceMap.Sample(LinearSampler, normal).rgb;
	float3 diffuse = irradiance;

	float3 ambient = (kD * diffuse);

	colour += ambient * occlusion * material.Albedo.rgb;
	colour += emissive;

	return float4( colour, material.Albedo.a );
}