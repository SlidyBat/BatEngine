#pragma once

#include "MathLib.h"

namespace Bat
{
	struct ShaderMaterial
	{
		Vec4  GlobalAmbient;

		Vec4  AmbientColor;

		Vec4  EmissiveColor;

		Vec4  DiffuseColor;

		Vec4  SpecularColor;

		Vec4  Reflectance;

		float   Opacity;
		float   SpecularPower;
		float   IndexOfRefraction;
		int     HasAmbientTexture;

		int     HasEmissiveTexture;
		int     HasDiffuseTexture;
		int     HasSpecularTexture;
		int     HasSpecularPowerTexture;

		int     HasNormalTexture;
		int     HasBumpTexture;
		int     HasOpacityTexture;
		float   BumpIntensity;

		float   SpecularScale;
		float   AlphaThreshold;
		float   _pad1[2];
	};

	constexpr size_t MAX_LIGHTS = 16;
	struct ShaderLight
	{
		Vec3       Position;
		float      _pad0;

		Vec3       Diffuse;
		float      _pad2;

		Vec3       Specular;
		float      _pad3;

		float      Range;
		uint32_t   Type;
		float      _pad4[2];
	};
}