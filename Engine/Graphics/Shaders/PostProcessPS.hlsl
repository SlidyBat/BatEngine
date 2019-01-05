#include "Common.hlsli"

#define BLACK_AND_WHITE
#define LINES_AND_FLICKER
#define BLOTCHES
#define GRAIN

#define FREQUENCY 15.0

Texture2D shaderTexture;

cbuffer Globals
{
    float2 resolution;
    float time;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float rand(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float rand(float c)
{
    return rand(float2(c, 1.0));
}

float randomLine(float2 uv, float seed)
{
    float b = 0.01 * rand(seed);
    float a = rand(seed + 1.0);
    float c = rand(seed + 2.0) - 0.5;
    float mu = rand(seed + 3.0);
	
    float l = 1.0;
	
    if (mu > 0.2)
        l = pow(abs(a * uv.x + b * uv.y + c), 1.0 / 8.0);
    else
        l = 2.0 - pow(abs(a * uv.x + b * uv.y + c), 1.0 / 8.0);
	
    return lerp(0.5, 1.0, l);
}

// Generate some blotches.
float randomBlotch(float2 uv, float seed)
{
    float x = rand(seed);
    float y = rand(seed + 1.0);
    float s = 0.01 * rand(seed + 2.0);
	
    float2 p = float2(x, y) - uv;
    p.x *= resolution.x / resolution.y;
    float a = atan2(p.y, p.x);
    float v = 1.0;
    float ss = s * s * (sin(6.2831 * a * x) * 0.1 + 1.0);
	
    if (dot(p, p) < ss)
        v = 0.2;
    else
        v = pow(dot(p, p) - ss, 1.0 / 16.0);
	
    return lerp(0.3 + 0.2 * (1.0 - (s / 0.02)), 1.0, v);
}


float4 main(PixelInputType input) : SV_TARGET
{
    float xPixelWidth = 6 * (1.0f / resolution.x);
    float yPixelWidth = 6 * (1.0f / resolution.y);
    float xModTex = input.tex.x - fmod(input.tex.x, xPixelWidth);
    float yModTex = input.tex.y - fmod(input.tex.y, yPixelWidth);

    return shaderTexture.Sample(ClampSampler, float2(xModTex, yModTex));
}