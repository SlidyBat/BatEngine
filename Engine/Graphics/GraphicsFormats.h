#pragma once

#include "Core/Common.h"
#include <limits>

namespace Bat
{
	enum TexFormat
	{
		TEX_FORMAT_UNKNOWN	                    = 0,
		TEX_FORMAT_R32G32B32A32_TYPELESS       = 1,
		TEX_FORMAT_R32G32B32A32_FLOAT          = 2,
		TEX_FORMAT_R32G32B32A32_UINT           = 3,
		TEX_FORMAT_R32G32B32A32_SINT           = 4,
		TEX_FORMAT_R32G32B32_TYPELESS          = 5,
		TEX_FORMAT_R32G32B32_FLOAT             = 6,
		TEX_FORMAT_R32G32B32_UINT              = 7,
		TEX_FORMAT_R32G32B32_SINT              = 8,
		TEX_FORMAT_R16G16B16A16_TYPELESS       = 9,
		TEX_FORMAT_R16G16B16A16_FLOAT          = 10,
		TEX_FORMAT_R16G16B16A16_UNORM          = 11,
		TEX_FORMAT_R16G16B16A16_UINT           = 12,
		TEX_FORMAT_R16G16B16A16_SNORM          = 13,
		TEX_FORMAT_R16G16B16A16_SINT           = 14,
		TEX_FORMAT_R32G32_TYPELESS             = 15,
		TEX_FORMAT_R32G32_FLOAT                = 16,
		TEX_FORMAT_R32G32_UINT                 = 17,
		TEX_FORMAT_R32G32_SINT                 = 18,
		TEX_FORMAT_R32G8X24_TYPELESS           = 19,
		TEX_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
		TEX_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
		TEX_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
		TEX_FORMAT_R10G10B10A2_TYPELESS        = 23,
		TEX_FORMAT_R10G10B10A2_UNORM           = 24,
		TEX_FORMAT_R10G10B10A2_UINT            = 25,
		TEX_FORMAT_R11G11B10_FLOAT             = 26,
		TEX_FORMAT_R8G8B8A8_TYPELESS           = 27,
		TEX_FORMAT_R8G8B8A8_UNORM              = 28,
		TEX_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
		TEX_FORMAT_R8G8B8A8_UINT               = 30,
		TEX_FORMAT_R8G8B8A8_SNORM              = 31,
		TEX_FORMAT_R8G8B8A8_SINT               = 32,
		TEX_FORMAT_R16G16_TYPELESS             = 33,
		TEX_FORMAT_R16G16_FLOAT                = 34,
		TEX_FORMAT_R16G16_UNORM                = 35,
		TEX_FORMAT_R16G16_UINT                 = 36,
		TEX_FORMAT_R16G16_SNORM                = 37,
		TEX_FORMAT_R16G16_SINT                 = 38,
		TEX_FORMAT_R32_TYPELESS                = 39,
		TEX_FORMAT_D32_FLOAT                   = 40,
		TEX_FORMAT_R32_FLOAT                   = 41,
		TEX_FORMAT_R32_UINT                    = 42,
		TEX_FORMAT_R32_SINT                    = 43,
		TEX_FORMAT_R24G8_TYPELESS              = 44,
		TEX_FORMAT_D24_UNORM_S8_UINT           = 45,
		TEX_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
		TEX_FORMAT_X24_TYPELESS_G8_UINT        = 47,
		TEX_FORMAT_R8G8_TYPELESS               = 48,
		TEX_FORMAT_R8G8_UNORM                  = 49,
		TEX_FORMAT_R8G8_UINT                   = 50,
		TEX_FORMAT_R8G8_SNORM                  = 51,
		TEX_FORMAT_R8G8_SINT                   = 52,
		TEX_FORMAT_R16_TYPELESS                = 53,
		TEX_FORMAT_R16_FLOAT                   = 54,
		TEX_FORMAT_D16_UNORM                   = 55,
		TEX_FORMAT_R16_UNORM                   = 56,
		TEX_FORMAT_R16_UINT                    = 57,
		TEX_FORMAT_R16_SNORM                   = 58,
		TEX_FORMAT_R16_SINT                    = 59,
		TEX_FORMAT_R8_TYPELESS                 = 60,
		TEX_FORMAT_R8_UNORM                    = 61,
		TEX_FORMAT_R8_UINT                     = 62,
		TEX_FORMAT_R8_SNORM                    = 63,
		TEX_FORMAT_R8_SINT                     = 64,
		TEX_FORMAT_A8_UNORM                    = 65,
		TEX_FORMAT_R1_UNORM                    = 66,
		TEX_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
		TEX_FORMAT_R8G8_B8G8_UNORM             = 68,
		TEX_FORMAT_G8R8_G8B8_UNORM             = 69,
		TEX_FORMAT_BC1_TYPELESS                = 70,
		TEX_FORMAT_BC1_UNORM                   = 71,
		TEX_FORMAT_BC1_UNORM_SRGB              = 72,
		TEX_FORMAT_BC2_TYPELESS                = 73,
		TEX_FORMAT_BC2_UNORM                   = 74,
		TEX_FORMAT_BC2_UNORM_SRGB              = 75,
		TEX_FORMAT_BC3_TYPELESS                = 76,
		TEX_FORMAT_BC3_UNORM                   = 77,
		TEX_FORMAT_BC3_UNORM_SRGB              = 78,
		TEX_FORMAT_BC4_TYPELESS                = 79,
		TEX_FORMAT_BC4_UNORM                   = 80,
		TEX_FORMAT_BC4_SNORM                   = 81,
		TEX_FORMAT_BC5_TYPELESS                = 82,
		TEX_FORMAT_BC5_UNORM                   = 83,
		TEX_FORMAT_BC5_SNORM                   = 84,
		TEX_FORMAT_B5G6R5_UNORM                = 85,
		TEX_FORMAT_B5G5R5A1_UNORM              = 86,
		TEX_FORMAT_B8G8R8A8_UNORM              = 87,
		TEX_FORMAT_B8G8R8X8_UNORM              = 88,
		TEX_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
		TEX_FORMAT_B8G8R8A8_TYPELESS           = 90,
		TEX_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
		TEX_FORMAT_B8G8R8X8_TYPELESS           = 92,
		TEX_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
		TEX_FORMAT_BC6H_TYPELESS               = 94,
		TEX_FORMAT_BC6H_UF16                   = 95,
		TEX_FORMAT_BC6H_SF16                   = 96,
		TEX_FORMAT_BC7_TYPELESS                = 97,
		TEX_FORMAT_BC7_UNORM                   = 98,
		TEX_FORMAT_BC7_UNORM_SRGB              = 99,
		TEX_FORMAT_AYUV                        = 100,
		TEX_FORMAT_Y410                        = 101,
		TEX_FORMAT_Y416                        = 102,
		TEX_FORMAT_NV12                        = 103,
		TEX_FORMAT_P010                        = 104,
		TEX_FORMAT_P016                        = 105,
		TEX_FORMAT_420_OPAQUE                  = 106,
		TEX_FORMAT_YUY2                        = 107,
		TEX_FORMAT_Y210                        = 108,
		TEX_FORMAT_Y216                        = 109,
		TEX_FORMAT_NV11                        = 110,
		TEX_FORMAT_AI44                        = 111,
		TEX_FORMAT_IA44                        = 112,
		TEX_FORMAT_P8                          = 113,
		TEX_FORMAT_A8P8                        = 114,
		TEX_FORMAT_B4G4R4A4_UNORM              = 115,

		TEX_FORMAT_P208                        = 130,
		TEX_FORMAT_V208                        = 131,
		TEX_FORMAT_V408                        = 132
	};

	struct TexFormatInfo_t
	{
		const char* name;
		int num_bytes;
		int num_red_bits;
		int num_green_bits;
		int num_blue_bits;
		int num_alpha_bits;
		bool compressed;
	};

	const TexFormatInfo_t& TexFormatInfo( TexFormat format );

	enum class TextureAddressMode
	{
		WRAP	= 1,
		MIRROR	= 2,
		CLAMP	= 3,
		BORDER	= 4,
		MIRROR_ONCE	= 5
	};

	enum class ComparisonFunc
	{
		NEVER	= 1,
		LESS	= 2,
		EQUAL	= 3,
		LESS_EQUAL	= 4,
		GREATER	= 5,
		NOT_EQUAL	= 6,
		GREATER_EQUAL	= 7,
		ALWAYS	= 8
	};

	enum class SampleFilter
	{
		MIN_MAG_MIP_POINT	= 0,
		MIN_MAG_POINT_MIP_LINEAR	= 0x1,
		MIN_POINT_MAG_LINEAR_MIP_POINT	= 0x4,
		MIN_POINT_MAG_MIP_LINEAR	= 0x5,
		MIN_LINEAR_MAG_MIP_POINT	= 0x10,
		MIN_LINEAR_MAG_POINT_MIP_LINEAR	= 0x11,
		MIN_MAG_LINEAR_MIP_POINT	= 0x14,
		MIN_MAG_MIP_LINEAR	= 0x15,
		ANISOTROPIC	= 0x55,
		COMPARISON_MIN_MAG_MIP_POINT	= 0x80,
		COMPARISON_MIN_MAG_POINT_MIP_LINEAR	= 0x81,
		COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT	= 0x84,
		COMPARISON_MIN_POINT_MAG_MIP_LINEAR	= 0x85,
		COMPARISON_MIN_LINEAR_MAG_MIP_POINT	= 0x90,
		COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR	= 0x91,
		COMPARISON_MIN_MAG_LINEAR_MIP_POINT	= 0x94,
		COMPARISON_MIN_MAG_MIP_LINEAR	= 0x95,
		COMPARISON_ANISOTROPIC	= 0xd5,
		MINIMUM_MIN_MAG_MIP_POINT	= 0x100,
		MINIMUM_MIN_MAG_POINT_MIP_LINEAR	= 0x101,
		MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT	= 0x104,
		MINIMUM_MIN_POINT_MAG_MIP_LINEAR	= 0x105,
		MINIMUM_MIN_LINEAR_MAG_MIP_POINT	= 0x110,
		MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR	= 0x111,
		MINIMUM_MIN_MAG_LINEAR_MIP_POINT	= 0x114,
		MINIMUM_MIN_MAG_MIP_LINEAR	= 0x115,
		MINIMUM_ANISOTROPIC	= 0x155,
		MAXIMUM_MIN_MAG_MIP_POINT	= 0x180,
		MAXIMUM_MIN_MAG_POINT_MIP_LINEAR	= 0x181,
		MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT	= 0x184,
		MAXIMUM_MIN_POINT_MAG_MIP_LINEAR	= 0x185,
		MAXIMUM_MIN_LINEAR_MAG_MIP_POINT	= 0x190,
		MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR	= 0x191,
		MAXIMUM_MIN_MAG_LINEAR_MIP_POINT	= 0x194,
		MAXIMUM_MIN_MAG_MIP_LINEAR	= 0x195,
		MAXIMUM_ANISOTROPIC	= 0x1d5
	};

	enum class ShaderType
	{
		INVALID = -1,
		PIXEL = 0,
		VERTEX,
		GEOMETRY,
		HULL,
		COMPUTE
	};

	enum class GPUResourceUsage
	{
		DEFAULT	= 0,
		IMMUTABLE	= 1,
		DYNAMIC	= 2,
		STAGING	= 3
	};

	enum class PrimitiveTopology
	{
		INVALID = -1,
		POINTLIST = 0,
		LINELIST,
		LINESTRIP,
		TRIANGLELIST,
		TRIANGLESTRIP
	};

	enum ClearFlag
	{
		CLEAR_FLAG_DEPTH = 1,
		CLEAR_FLAG_STENCIL = 2
	};

	enum class CullMode
	{
		NONE = 1,
		FRONT = 2,
		BACK = 3
	};

	struct SamplerDesc
	{
		SampleFilter filter = SampleFilter::MIN_MAG_MIP_POINT;
		float mip_lod_bias = 0.0f;
		size_t max_anisotropy = 1;
		ComparisonFunc comparison_func = ComparisonFunc::ALWAYS;
		float border_color[4];
		float min_lod = 0.0f;
		float max_lod = std::numeric_limits<float>::max();
		TextureAddressMode address_u = TextureAddressMode::WRAP;
		TextureAddressMode address_v = TextureAddressMode::WRAP;
		TextureAddressMode address_w = TextureAddressMode::WRAP;
	};

	struct DeviceInfo
	{
		std::string name;
		size_t memory;
	};

	struct Viewport
	{
		float width = 0.0f;
		float height = 0.0f;
		float min_depth = 0.0f;
		float max_depth = 0.0f;
		Vec2 top_left = { 0.0f, 0.0f };
	};

	enum class StencilOp
	{
		KEEP = 1,
		ZERO = 2,
		REPLACE = 3,
		INCR_SAT = 4,
		DECR_SAT = 5,
		INVERT = 6,
		INCR = 7,
		DECR = 8
	};

	struct StencilOpDesc
	{
		StencilOp fail_op;
		StencilOp depth_fail_op;
		StencilOp pass_op;
		ComparisonFunc func;
	};

	enum class MsaaQuality
	{
		NONE = 0,
		STANDARD_PATTERN = -1,
		CENTER_PATTERN = -2
	};

	enum class TexFlags
	{
		NONE = 0,
		NO_SHADER_BIND = (1 << 0), // This resource won't be bound to a shader
		NO_GEN_MIPS = (1 << 1) // Won't generate mipmaps for this texture
	};
	BAT_ENUM_OPERATORS( TexFlags );
}