#pragma once

#include "MathLib.h"

namespace Bat
{
	struct ShaderMaterial
	{
		Vec4 BaseColourFactor;

		Vec3 EmissiveFactor;
		float MetallicFactor;

		float RoughnessFactor;
		int HasBaseColourTexture;
		int HasMetallicRoughnessTexture;
		int HasNormalTexture;

		int HasOcclusionTexture;
		int HasEmissiveTexture;
		float AlphaCutoff;
		float _pad0;
	};

	constexpr size_t MAX_LIGHTS = 16;
	struct ShaderLight
	{
		Vec3  Position;
		int ShadowIndex;

		Vec3   Direction;
		float  SpotlightAngle;

		Vec3  Colour;
		float _pad3;

		float Range;
		float Intensity;
		int   Type;
		float Pad;
	};
}