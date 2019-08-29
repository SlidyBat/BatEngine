#include "CommonPS.hlsli"

Texture2D DiffuseTexture : register(T_SLOT_0);
Texture2D SpecularTexture : register(T_SLOT_1);
Texture2D EmissiveTexture : register(T_SLOT_2);
Texture2D NormalTexture : register(T_SLOT_3);

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
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	float2 tex : TEXCOORD;
};

float3 DoPointLight( Light light, Material material, float3 normal, float3 world_pos )
{
	float3 light_dir = light.Position - world_pos;
	float light_distance = length( light_dir );
	light_dir /= light_distance; // normalize

	float attenuation = 1.0f - smoothstep( light.Range * 0.75f, light.Range, light_distance );

	// diffuse
	float diff = saturate( dot( normal, light_dir ) );
	float3 diffuse = light.Colour * (diff * material.DiffuseColor.rgb) * attenuation * light.Intensity;

	// specular
	float3 viewDir = normalize( Globals.CameraPos - world_pos );
	float3 reflectDir = reflect( -light_dir, normal );
	float spec = pow( saturate( dot( viewDir, reflectDir ) ), material.SpecularPower );
	float3 specular = light.Colour * (spec * material.SpecularColor.rgb) * attenuation * light.Intensity;

	float3 final = diffuse + specular;

	return final;
}

float3 DoDirectionalLight( Light light, Material material, float3 normal, float3 world_pos )
{
	float3 light_dir = normalize( -light.Direction ).xyz;

	// diffuse
	float diff = saturate( dot( normal, light_dir ) );
	float3 diffuse = light.Colour * (diff * material.DiffuseColor.rgb) * light.Intensity;

	// specular
	float3 viewDir = normalize( Globals.CameraPos - world_pos );
	float3 reflectDir = reflect( -light_dir, normal );
	float spec = pow( saturate( dot( viewDir, reflectDir ) ), material.SpecularPower );
	float3 specular = light.Colour * (spec * material.SpecularColor.rgb) * light.Intensity;

	float3 final = diffuse + specular;

	return final;
}

float3 DoSpotLight( Light light, Material material, float3 normal, float3 world_pos )
{
	float3 light_dir = light.Position - world_pos;
	float light_distance = length( light_dir );
	light_dir /= light_distance; // normalize

	float attenuation = 1.0f - smoothstep( light.Range * 0.75f, light.Range, light_distance );

	float min_theta = cos( light.SpotlightAngle );
	float max_theta = lerp( min_theta, 1, 0.5f );
	float theta = dot( light.Direction.xyz, -light_dir );
	float spot_intensity = smoothstep( min_theta, max_theta, theta );

	// diffuse
	float diff = saturate( dot( normal, light_dir ) );
	float3 diffuse = light.Colour * (diff * material.DiffuseColor.rgb) * light.Intensity * spot_intensity;

	// specular
	float3 viewDir = normalize( Globals.CameraPos - world_pos );
	float3 reflectDir = reflect( -light_dir, normal );
	float spec = pow( saturate( dot( viewDir, reflectDir ) ), material.SpecularPower );
	float3 specular = light.Colour * (spec * material.SpecularColor.rgb) * light.Intensity * spot_intensity;

	float3 final = diffuse + specular;

	return final;
}

float3 DoLight( Light light, Material material, float3 normal, float3 world_pos )
{
	switch( light.Type )
	{
		case POINT_LIGHT:
			return DoPointLight( light, material, normal, world_pos );
		case DIRECTIONAL_LIGHT:
			return DoDirectionalLight( light, material, normal, world_pos );
		case SPOT_LIGHT:
			return DoSpotLight( light, material, normal, world_pos );
		default:
			return float3(0.0f, 0.0f, 0.0f);
	}
}

float4 main( PixelInput input ) : SV_TARGET
{
	Material material = Mat;

	float3 normal = input.normal;
	if( material.HasNormalTexture )
	{
		normal = NormalTexture.Sample( WrapSampler, input.tex ).xyz;
		normal = normalize( (normal * 2.0f) - 1.0f );
		float3x3 tbn = float3x3(input.tangent, input.bitangent, input.normal);
		normal = normalize( mul( normal, tbn ) );
	}

	float3 world_pos = input.world_pos.xyz;
	float3 view_dir = normalize( Globals.CameraPos - world_pos );

	// Flip if facing away from camera (should only happen when backface culling disabled)
	if( dot( view_dir, normal ) < 0 )
	{
		normal = -normal;
	}

	// get specular colour of material
	if( material.HasSpecularTexture )
	{
		material.SpecularColor = SpecularTexture.Sample( WrapSampler, input.tex );
	}

	if( material.HasDiffuseTexture )
	{
		material.DiffuseColor = DiffuseTexture.Sample( WrapSampler, input.tex );
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
		colour += DoLight( Lights[i], material, normal, world_pos );
	}

	colour += ambient + emissive;

	return float4( colour, material.DiffuseColor.a * material.Opacity );
}