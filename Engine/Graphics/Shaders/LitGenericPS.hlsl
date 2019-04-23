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

float3 DoLight( Material material, float3 normal, float3 world_pos, Light light )
{
	float3 light_dir = light.Position - world_pos;
	float light_distance = length( light_dir );
	light_dir /= light_distance; // normalize

	float attenuation = 1.0f - smoothstep( light.Range * 0.75f, light.Range, light_distance );

	// diffuse
	float diff = saturate( dot( normal, light_dir ) );
	float3 diffuse = light.Diffuse * (diff * material.DiffuseColor.rgb) * attenuation;

	// specular
	float3 viewDir = normalize( Globals.CameraPos - world_pos );
	float3 reflectDir = reflect( -light_dir, normal );
	float spec = pow( saturate( dot( viewDir, reflectDir ) ), material.SpecularPower );
	float3 specular = light.Specular * (spec * material.SpecularColor.rgb) * attenuation;

	float3 final = diffuse + specular;
	
	return final;
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
		colour += DoLight( material, normal, input.world_pos.xyz, Lights[i] );
	}

	colour += ambient + emissive;

    return float4( colour, 1.0f );
}