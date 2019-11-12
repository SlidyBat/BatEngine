#include "CommonPS.hlsli"

#define NUM_SAMPLES 5

Texture2D SceneTexture : register(T_SLOT_0);
Texture2D DepthTexture : register(T_SLOT_1);

cbuffer Transform : register(B_SLOT_0)
{
    float4x4 inv_viewproj;
    float4x4 prev_viewproj;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    // https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch27.html

    // Get the depth buffer value at this pixel.
    float zOverW = DepthTexture.Sample(MirrorSampler, input.tex).r;
    // H is the viewport position at this pixel in the range -1 to 1.
    float4 H = float4(input.tex.x * 2 - 1, (1 - input.tex.y) * 2 - 1, zOverW, 1);
    // Transform by the view-projection inverse.
    float4 D = mul(H, inv_viewproj);
    // Divide by w to get the world position.
    float4 worldPos = D / D.w;
    
    // Current viewport position
    float4 currentPos = H;
    // Use the world position, and transform by the previous
    // view-projection matrix.
    float4 previousPos = mul(worldPos, prev_viewproj);
    // Convert to nonhomogeneous points [-1,1] by dividing by w.
    previousPos /= previousPos.w;
    // Use this frame's position and last frame's to compute the pixel
    // velocity.
    float2 velocity = (currentPos.xy - previousPos.xy) / 2.f;
    
    // Get the initial color at this pixel.
    float4 color = SceneTexture.Sample( MirrorSampler, input.tex);
    input.tex += velocity;
    for (int i = 1; i < NUM_SAMPLES; ++i, input.tex += velocity)
    {
        // Sample the color buffer along the velocity vector.
        float4 currentColor = SceneTexture.Sample(MirrorSampler, input.tex);
        // Add the current color to our color sum.
        color += currentColor;
    }

    // Average all of the samples to get the final blur color.
    return color / NUM_SAMPLES;
}