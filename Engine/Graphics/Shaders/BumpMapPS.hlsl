Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D emissiveTexture : register(t2);
Texture2D bumpMap : register(t3);
SamplerState SampleType;

cbuffer LightingParameters
{
    float3 cameraPos;
    float time;
    float shininess;
};

cbuffer Light
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
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 tex : TEXCOORD;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float3 lightDir = normalize(lightPosition - (float3) input.world_pos);
    float3 objDiffuse = diffuseTexture.Sample(SampleType, input.tex).xyz;
    float3 objSpecular = specularTexture.Sample(SampleType, input.tex).xyz;
    float3 objEmissive = emissiveTexture.Sample(SampleType, input.tex).xyz;
    float3 normal = bumpMap.Sample(SampleType, input.tex).xyz;
    normal = normalize((normal * 2.0f) - 1.0f);
    float3x3 tbn = float3x3(input.tangent, input.bitangent, input.normal);
    //normal = normalize(mul(normal, tbn));

    // ambient
    float3 ambient = lightAmbient * objDiffuse;
    
    // diffuse
    float diff = saturate(dot(normal, lightDir));
    float3 diffuse = lightDiffuse * (diff * objDiffuse);

    // specular
    float3 viewDir = normalize(cameraPos - (float3) input.world_pos);
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(saturate(dot(viewDir, reflectDir)), 32.0f);
    float3 specular = lightSpecular * (spec * objSpecular);


    return float4(ambient + diffuse + specular + objEmissive, 1.0f);
}