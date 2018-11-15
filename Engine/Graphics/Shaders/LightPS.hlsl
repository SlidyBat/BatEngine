Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightingParameters
{
    float3 cameraPos;
    float time;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float x = 5.0f + sin(time) * 10.0f;
    float y = sin(time / 2.0f) * 5.0f;
    float3 lightPos = float3(x, y, 0.0f);
    float3 lightDir = normalize(lightPos - (float3) input.normal);
    float3 lightColour = float3(1.0f, 1.0f, 1.0f);
    float4 objColour = shaderTexture.Sample(SampleType, input.tex);

    // ambient
    float ambientStrength = 0.2f;
    float3 ambient = ambientStrength * lightColour;

    // diffuse
    float diff = max(dot((float3) input.normal, lightDir), 0.0f);
    float3 diffuse = diff * lightColour;

    // specular
    float specularStrength = 0.5f;
    float3 viewDir = normalize(cameraPos - (float3)input.position);
    float3 reflectDir = reflect(-lightDir, (float3)input.normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    float3 specular = specularStrength * spec * lightColour;


    return float4((ambient + diffuse + specular) * (float3) objColour, 1.0f);
}