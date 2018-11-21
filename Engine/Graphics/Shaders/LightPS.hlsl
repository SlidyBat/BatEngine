Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightingParameters
{
    float3 cameraPos;
    float time;
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
    float4 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float3 lightDir = normalize(lightPosition - (float3) input.world_pos);
    float4 objColour = shaderTexture.Sample(SampleType, input.tex);

    // ambient
    float3 ambient = lightAmbient;

    input.normal = normalize(input.normal);
    // diffuse
    float diff = max(dot((float3) input.normal, lightDir), 0.0f);
    float3 diffuse = diff * lightDiffuse;

    // specular
    float3 viewDir = normalize(cameraPos - (float3) input.world_pos);
    float3 reflectDir = reflect(-lightDir, (float3)input.normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    float3 specular = spec * lightSpecular;


    return float4((ambient + diffuse + specular) * (float3)objColour, 1.0f);
}