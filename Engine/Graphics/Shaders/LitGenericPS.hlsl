#include "CommonPS.hlsli"

Texture2D DiffuseTexture : register(T_SLOT_0);
Texture2D SpecularTexture : register(T_SLOT_1);
Texture2D EmissiveTexture : register(T_SLOT_2);
Texture2D NormalTexture : register(T_SLOT_3);
Texture2DArray ShadowMap : register(T_SLOT_SHADOWMAPS);

cbuffer Material : register(B_SLOT_0)
{
	Material Mat;
}

cbuffer LightParams : register(B_SLOT_1)
{
	uint NumLights;
	Light Lights[MAX_LIGHTS];
}

cbuffer Matrices : register(B_SLOT_2)
{
	float4x4 ShadowMatrix[MAX_SHADOW_SOURCES];
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

float Shadow( Light light, float3 pos_ws, float3 normal, float3 light_dir )
{
	if( light.ShadowIndex == INVALID_SHADOW_MAP_INDEX )
	{
		return 1.0f;
	}
	
	float4 proj_coords = mul( float4( pos_ws, 1.0f ), ShadowMatrix[light.ShadowIndex] );
	proj_coords.xyz /= proj_coords.w;
	
	float bias = 0.0005f;
	float current_depth = proj_coords.z - bias;
	
	float2 shadow_uv = proj_coords.xy * float2( 0.5f, -0.5f ) + float2( 0.5f, 0.5f );
	
	int range = 2;
	float shadow = 0.0f;
	[unroll]
	for (int y = -range; y <= range; y++)
	{
		for (int x = -range; x <= range; x++)
		{
			shadow += ShadowMap.SampleCmpLevelZero(CompareDepthSampler, float3(shadow_uv, light.ShadowIndex), current_depth, int2(x, y)).r;
		}
	}
	shadow /= (range * 2 + 1) * (range * 2 + 1);

	return shadow;
}

float3 DoDiffuse( Light light, Material material, float3 light_dir, float3 n )
{
	return material.DiffuseColor.rgb * light.Colour * light.Intensity * max( 0.0f, dot( light_dir, n ) );
}

float3 DoSpecular( Light light, Material material, float3 world_pos, float3 light_dir, float3 n )
{
	float3 r = reflect( light_dir, n );
	float3 v = Globals.CameraPos - world_pos;
	return material.SpecularColor.rgb * light.Colour * light.Intensity * pow( max( 0.0f, dot( normalize( -r ), normalize( v ) ) ), material.SpecularPower );
}

float3 DoPointLight( Light light, Material material, float3 world_pos, float3 n )
{
	float3 light_dir = light.Position - world_pos;
	float light_distance = length( light_dir );
	light_dir /= light_distance;

	float attenuation = 1.0f - smoothstep( light.Range * 0.75f, light.Range, light_distance );

	float3 diffuse = DoDiffuse( light, material, light_dir, n );
	float3 specular = DoSpecular( light, material, world_pos, light_dir, n );

	return (diffuse + specular) * attenuation;
}

float3 DoSpotLight( Light light, Material material, float3 world_pos, float3 n )
{
	float3 light_dir = light.Position - world_pos;
	float light_distance = length( light_dir );
	light_dir /= light_distance;

	float min_theta = cos( light.SpotlightAngle );
	float max_theta = lerp( min_theta, 1, 0.5f );
	float theta = dot( light.Direction.xyz, -light_dir );
	float spot_intensity = smoothstep( min_theta, max_theta, theta );

	float attenuation = 1.0f - smoothstep( light.Range * 0.75f, light.Range, light_distance );

	float3 diffuse = DoDiffuse( light, material, light_dir, n );
	float3 specular = DoSpecular( light, material, world_pos, light_dir, n );

	return (diffuse + specular) * attenuation * spot_intensity * Shadow( light, world_pos, n, light_dir );
}

float3 DoDirectionalLight( Light light, Material material, float3 world_pos, float3 n )
{
	float3 light_dir = normalize( -light.Direction ).xyz;

	float3 diffuse = DoDiffuse( light, material, light_dir, n );
	float3 specular = DoSpecular( light, material, world_pos, light_dir, n );

	return (diffuse + specular) * Shadow( light, world_pos, n, light_dir );
}

float3 DoLight( Light light, Material material, float3 world_pos, float3 normal )
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
	Material material = Mat;

	float3 normal = input.normal;
#ifdef HAS_TANGENT
	if( material.HasNormalTexture )
	{
		float3 normal_sample = NormalTexture.Sample( WrapSampler, input.tex ).xyz;
		normal = normal_sample * 2.0f - 1.0f;
		normal.y = -normal.y;
		
		float3x3 tbn = float3x3(normalize(input.tangent), normalize(input.bitangent), normalize(input.normal));
		normal = normalize( mul( normal, tbn ) );
	}
#endif

	normal = normalize( normal );

	float3 world_pos = input.world_pos.xyz;
	float3 view_dir = normalize( Globals.CameraPos - world_pos );

	// get specular colour of material
	if( material.HasSpecularTexture )
	{
		material.SpecularColor = SpecularTexture.Sample( WrapSampler, input.tex );
	}

	if( material.HasDiffuseTexture )
	{
		material.DiffuseColor = DiffuseTexture.Sample(WrapSampler, input.tex);
		material.DiffuseColor = ToLinearSpace(material.DiffuseColor);
	}
	

	// ambient
	float3 ambient = material.AmbientColor.rgb;
	if( material.HasAmbientTexture )
	{
		if( any( ambient ) )
		{
			ambient *= material.AmbientColor.rgb;
		}
		else
		{
			ambient = material.AmbientColor.rgb;
		}
	}

	ambient *= material.GlobalAmbient.rgb;
	ambient *= material.DiffuseColor.rgb;

	// emissive
	float3 emissive = material.EmissiveColor.rgb;
	if( material.HasEmissiveTexture )
	{
		float3 sample = EmissiveTexture.Sample(WrapSampler, input.tex).rgb;
		sample = ToLinearSpace(sample);
		
		if( any( emissive ) )
		{
			emissive *= EmissiveTexture.Sample( WrapSampler, input.tex ).rgb;
		}
		else
		{
			emissive = EmissiveTexture.Sample( WrapSampler, input.tex ).rgb;
		}
	}

	float3 colour = 0.0f;
	for( uint i = 0; i < NumLights; i++ )
	{
		colour += DoLight( Lights[i], material, world_pos, normal );
	}

	colour += ambient + emissive;

	return float4( colour, material.DiffuseColor.a * material.Opacity );
}