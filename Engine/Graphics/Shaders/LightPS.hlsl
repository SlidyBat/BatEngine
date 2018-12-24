Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D emissiveTexture : register(t2);
SamplerState SampleType;

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
    float3 objDiffuse = diffuseTexture.Sample(SampleType, input.tex).xyz;
    float3 objSpecular = specularTexture.Sample(SampleType, input.tex).xyz;
    float3 objEmissive = emissiveTexture.Sample(SampleType, input.tex).xyz;

    // ambient
    float3 ambient = lightAmbient * objDiffuse;

    float3 normal = normalize(input.normal);
    // diffuse
    float diff = saturate(dot(normal, lightDir));
    float3 diffuse = lightDiffuse * (diff * objDiffuse);

    // specular
    float3 viewDir = normalize(cameraPos - world_pos);
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f);
    float3 specular = lightSpecular * (spec * objSpecular);


    return float4(ambient + diffuse + specular + objEmissive, 1.0f);
}