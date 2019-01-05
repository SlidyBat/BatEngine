#include "Common.hlsli"

Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D emissiveTexture : register(t2);

cbuffer LightingParameters : register(b0)
{
    float3 cameraPos;
    float time;
    float shininess;
};

cbuffer Light : register(b1)
{
    float3 lightPosition;
    float3 lightAmbient;
    float3 lightDiffuse;
    float3 lightSpecular;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 world_pos : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float3 world_pos = (float3) input.world_pos;

    float3 lightDir = normalize(lightPosition - world_pos);
    float lightToPixelLength = length(lightPosition - world_pos);
    float attenuation = rcp(1.0 + 0.0007* lightToPixelLength + 0.00002 * lightToPixelLength * lightToPixelLength);
    float3 objDiffuse = diffuseTexture.Sample(WrapSampler, input.tex).xyz;
    float3 objSpecular = specularTexture.Sample(WrapSampler, input.tex).xyz;
    float3 objEmissive = emissiveTexture.Sample(WrapSampler, input.tex).xyz;

    // ambient
    float3 ambient = lightAmbient * objDiffuse;

    float3 normal = normalize(input.normal);
    // diffuse
    float diff = saturate(dot(normal, lightDir));
    float3 diffuse = lightDiffuse * (diff * objDiffuse) * attenuation;

    // specular
    float3 viewDir = normalize(cameraPos - world_pos);
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f);
    float3 specular = lightSpecular * (spec * objSpecular) * attenuation;

    float3 final = ambient + diffuse + specular + objEmissive;

    return float4(final, 1.0f);
}