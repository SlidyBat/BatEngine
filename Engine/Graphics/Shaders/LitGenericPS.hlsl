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

float3 DoLight( PixelInput input, float3 normal, Light light )
{
	float3 light_dir = light.Position - (float3)input.world_pos;
	float light_distance = length( light_dir );
	light_dir /= light_distance; // normalize

	float attenuation = 1.0f - smoothstep( light.Range * 0.75f, light.Range, light_distance );

	float3 obj_diffuse = Mat.DiffuseColor.rgb;
	if( Mat.HasDiffuseTexture )
	{
		if( any( obj_diffuse ) )
		{
			obj_diffuse *= DiffuseTexture.Sample( WrapSampler, input.tex ).rgb;
		}
		else
		{
			obj_diffuse = DiffuseTexture.Sample( WrapSampler, input.tex ).rgb;
		}
	}

	float3 obj_specular = Mat.SpecularColor.rgb;
	if( Mat.HasSpecularTexture )
	{
		if( any( obj_specular ) )
		{
			obj_specular *= SpecularTexture.Sample( WrapSampler, input.tex ).rgb;
		}
		else
		{
			obj_specular = SpecularTexture.Sample( WrapSampler, input.tex ).rgb;
		}
	}

	// diffuse
	float diff = saturate( dot( normal, light_dir ) );
	float3 diffuse = light.Diffuse * (diff * obj_diffuse) * attenuation;

	// specular
	float3 viewDir = normalize( Globals.CameraPos - (float3)input.world_pos );
	float3 reflectDir = reflect( -light_dir, normal );
	float spec = pow( saturate( dot( viewDir, reflectDir ) ), Mat.SpecularPower );
	float3 specular = light.Specular * (spec * obj_specular) * attenuation;

	float3 final = diffuse + specular;
	
	return final;
}

float4 main( PixelInput input ) : SV_TARGET
{
	float3 world_pos = (float3)input.world_pos;

	float3 normal = input.normal;
	if( Mat.HasNormalTexture )
	{
		normal = NormalTexture.Sample( WrapSampler, input.tex ).xyz;
		normal = normalize( (normal * 2.0f) - 1.0f );
		float3x3 tbn = float3x3(input.tangent, input.bitangent, input.normal);
		normal = normalize( mul( normal, tbn ) );
	}

	// ambient
	float3 ambient = Mat.AmbientColor.rgb;
	if( Mat.HasAmbientTexture )
	{
		if( any( ambient ) )
		{
			ambient *= Mat.AmbientColor.rgb;
		}
		else
		{
			ambient = Mat.AmbientColor.rgb;
		}
	}

	ambient *= Mat.GlobalAmbient;

	// emissive
	float3 emissive = Mat.EmissiveColor.rgb;
	if( Mat.HasEmissiveTexture )
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
		colour += DoLight( input, normal, Lights[i] );
	}

	colour += ambient + emissive;

    return float4( colour, 1.0f );
}