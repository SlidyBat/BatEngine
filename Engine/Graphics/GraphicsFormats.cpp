#include "PCH.h"
#include "GraphicsFormats.h"

namespace Bat
{
	static TexFormatInfo_t g_TexFormatInfo[] =
	{
		{ "UNKNOWN", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_UNKNOWN
		{ "R32G32B32A32_TYPELESS", 16, 32, 32, 32, 32, false }, // TEX_FORMAT_R32G32B32A32_TYPELESS
		{ "R32G32B32A32_FLOAT", 16, 32, 32, 32, 32, false }, // TEX_FORMAT_R32G32B32A32_FLOAT
		{ "R32G32B32A32_UINT", 16, 32, 32, 32, 32, false }, // TEX_FORMAT_R32G32B32A32_UINT
		{ "R32G32B32A32_SINT", 16, 32, 32, 32, 32, false }, // TEX_FORMAT_R32G32B32A32_SINT
		{ "R32G32B32_TYPELESS",12, 32, 32, 32, 0, false }, // TEX_FORMAT_R32G32B32_TYPELESS
		{ "R32G32B32_FLOAT", 12, 32, 32, 32, 0, false }, // TEX_FORMAT_R32G32B32_FLOAT
		{ "R32G32B32_UINT", 12, 32, 32, 32, 0, false }, // TEX_FORMAT_R32G32B32_UINT
		{ "R32G32B32_SINT", 12, 32, 32, 32, 0, false }, // TEX_FORMAT_R32G32B32_SINT
		{ "R16G16B16A16_TYPELESS", 8, 16, 16, 16, 16, false }, // TEX_FORMAT_R16G16B16A16_TYPELESS
		{ "R16G16B16A16_FLOAT", 8, 16, 16, 16, 16, false }, // TEX_FORMAT_R16G16B16A16_FLOAT
		{ "R16G16B16A16_UNORM", 8, 16, 16, 16, 16, false }, // TEX_FORMAT_R16G16B16A16_UNORM
		{ "R16G16B16A16_UINT", 8, 16, 16, 16, 16, false }, // TEX_FORMAT_R16G16B16A16_UINT
		{ "R16G16B16A16_SNORM", 8, 16, 16, 16, 16, false }, // TEX_FORMAT_R16G16B16A16_SNORM
		{ "R16G16B16A16_SINT", 8, 16, 16, 16, 16, false }, // TEX_FORMAT_R16G16B16A16_SINT
		{ "R32G32_TYPELESS", 8, 32, 32, 0, 0, false }, // TEX_FORMAT_R32G32_TYPELESS
		{ "R32G32_FLOAT", 8, 32, 32, 0, 0, false }, // TEX_FORMAT_R32G32_FLOAT
		{ "R32G32_UINT", 8, 32, 32, 0, 0, false }, // TEX_FORMAT_R32G32_UINT
		{ "R32G32_SINT", 8, 32, 32, 0, 0, false }, // TEX_FORMAT_R32G32_SINT
		{ "R32G8X24_TYPELESS", 8, 32, 8, 0, 24, false }, // TEX_FORMAT_R32G8X24_TYPELESS
		{ "D32_FLOAT_S8X24_UINT", 8, 0, 0, 0, 0, false }, // TEX_FORMAT_D32_FLOAT_S8X24_UINT
		{ "R32_FLOAT_X8X24_TYPELESS", 8, 32, 0, 0, 0, false }, // TEX_FORMAT_R32_FLOAT_X8X24_TYPELESS
		{ "X32_TYPELESS_G8X24_UINT", 8, 0, 8, 0, 0, false }, // TEX_FORMAT_X32_TYPELESS_G8X24_UINT
		{ "R10G10B10A2_TYPELESS", 4, 10, 10, 10, 2, false }, // TEX_FORMAT_R10G10B10A2_TYPELESS
		{ "R10G10B10A2_UNORM", 4, 10, 10, 10, 2, false }, // TEX_FORMAT_R10G10B10A2_UNORM
		{ "R10G10B10A2_UINT", 4, 10, 10, 10, 2, false }, // TEX_FORMAT_R10G10B10A2_UINT
		{ "R11G11B10_FLOAT", 4, 11, 11, 10, 0, false }, // TEX_FORMAT_R11G11B10_FLOAT
		{ "R8G8B8A8_TYPELESS", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_R8G8B8A8_TYPELESS
		{ "R8G8B8A8_UNORM", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_R8G8B8A8_UNORM
		{ "R8G8B8A8_UNORM_SRGB", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_R8G8B8A8_UNORM_SRGB
		{ "R8G8B8A8_UINT", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_R8G8B8A8_UINT
		{ "R8G8B8A8_SNORM", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_R8G8B8A8_SNORM
		{ "R8G8B8A8_SINT", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_R8G8B8A8_SINT
		{ "R16G16_TYPELESS", 4, 16, 16, 0, 0, false }, // TEX_FORMAT_R16G16_TYPELESS
		{ "R16G16_FLOAT", 4, 16, 16, 0, 0, false }, // TEX_FORMAT_R16G16_FLOAT
		{ "R16G16_UNORM", 4, 16, 16, 0, 0, false }, // TEX_FORMAT_R16G16_UNORM
		{ "R16G16_UINT", 4, 16, 16, 0, 0, false }, // TEX_FORMAT_R16G16_UINT
		{ "R16G16_SNORM", 4, 16, 16, 0, 0, false }, // TEX_FORMAT_R16G16_SNORM
		{ "R16G16_SINT", 4, 16, 16, 0, 0, false }, // TEX_FORMAT_R16G16_SINT
		{ "R32_TYPELESS", 4, 32, 0, 0, 0, false }, // TEX_FORMAT_R32_TYPELESS
		{ "D32_FLOAT", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_D32_FLOAT
		{ "R32_FLOAT", 4, 32, 0, 0, 0, false }, // TEX_FORMAT_R32_FLOAT
		{ "R32_UINT", 4, 32, 0, 0, 0, false }, // TEX_FORMAT_R32_UINT
		{ "R32_SINT", 4, 32, 0, 0, 0, false }, // TEX_FORMAT_R32_SINT
		{ "R24G8_TYPELESS", 4, 24, 8, 0, 0, false }, // TEX_FORMAT_R24G8_TYPELESS
		{ "D24_UNORM_S8_UINT", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_D24_UNORM_S8_UINT
		{ "R24_UNORM_X8_TYPELESS", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_R24_UNORM_X8_TYPELESS
		{ "X24_TYPELESS_G8_UINT", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_X24_TYPELESS_G8_UINT
		{ "R8G8_TYPELESS", 2, 8, 8, 0, 0, false }, // TEX_FORMAT_R8G8_TYPELESS
		{ "R8G8_UNORM", 2, 8, 8, 0, 0, false }, // TEX_FORMAT_R8G8_UNORM
		{ "R8G8_UINT", 2, 8, 8, 0, 0, false }, // TEX_FORMAT_R8G8_UINT
		{ "R8G8_SNORM", 2, 8, 8, 0, 0, false }, // TEX_FORMAT_R8G8_SNORM
		{ "R8G8_SINT", 2, 8, 8, 0, 0, false }, // TEX_FORMAT_R8G8_SINT
		{ "R16_TYPELESS", 2, 16, 0, 0, 0, false }, // TEX_FORMAT_R16_TYPELESS
		{ "R16_FLOAT", 2, 16, 0, 0, 0, false }, // TEX_FORMAT_R16_FLOAT
		{ "D16_UNORM", 2, 0, 0, 0, 0, false }, // TEX_FORMAT_D16_UNORM
		{ "R16_UNORM", 2, 16, 0, 0, 0, false }, // TEX_FORMAT_R16_UNORM
		{ "R16_UINT", 2, 16, 0, 0, 0, false }, // TEX_FORMAT_R16_UINT
		{ "R16_SNORM", 2, 16, 0, 0, 0, false }, // TEX_FORMAT_R16_SNORM
		{ "R16_SINT", 2, 16, 0, 0, 0, false }, // TEX_FORMAT_R16_SINT
		{ "R8_TYPELESS", 1, 8, 0, 0, 0, false }, // TEX_FORMAT_R8_TYPELESS
		{ "R8_UNORM", 1, 8, 0, 0, 0, false }, // TEX_FORMAT_R8_UNORM
		{ "R8_UINT", 1, 8, 0, 0, 0, false }, // TEX_FORMAT_R8_UINT
		{ "R8_SNORM", 1, 8, 0, 0, 0, false }, // TEX_FORMAT_R8_SNORM
		{ "R8_SINT", 1, 8, 0, 0, 0, false }, // TEX_FORMAT_R8_SINT
		{ "A8_UNORM", 1, 0, 0, 0, 8, false }, // TEX_FORMAT_A8_UNORM
		{ "R1_UNORM", 0, 1, 0, 0, 0, false }, // TEX_FORMAT_R1_UNORM
		{ "R9G9B9E5_SHAREDEXP", 4, 9, 9, 9, 0, false }, // TEX_FORMAT_R9G9B9E5_SHAREDEXP
		{ "R8G8_B8G8_UNORM", 4, 8, 16, 8, 0, false }, // TEX_FORMAT_R8G8_B8G8_UNORM
		{ "G8R8_G8B8_UNORM", 4, 8, 16, 8, 0, false }, // TEX_FORMAT_G8R8_G8B8_UNORM
		{ "BC1_TYPELESS", 8, 0, 0, 0, 0, true }, // TEX_FORMAT_BC1_TYPELESS
		{ "BC1_UNORM", 8, 0, 0, 0, 0, true }, // TEX_FORMAT_BC1_UNORM
		{ "BC1_UNORM_SRGB", 8, 0, 0, 0, 0, true }, // TEX_FORMAT_BC1_UNORM_SRGB
		{ "BC2_TYPELESS", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC2_TYPELESS
		{ "BC2_UNORM", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC2_UNORM
		{ "BC2_UNORM_SRGB", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC2_UNORM_SRGB
		{ "BC3_TYPELESS", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC3_TYPELESS
		{ "BC3_UNORM", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC3_UNORM
		{ "BC3_UNORM_SRGB", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC3_UNORM_SRGB
		{ "BC4_TYPELESS", 8, 0, 0, 0, 0, true }, // TEX_FORMAT_BC4_TYPELESS
		{ "BC4_UNORM", 8, 0, 0, 0, 0, true }, // TEX_FORMAT_BC4_UNORM
		{ "BC4_SNORM", 8, 0, 0, 0, 0, true }, // TEX_FORMAT_BC4_SNORM
		{ "BC5_TYPELESS", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC5_TYPELESS
		{ "BC5_UNORM", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC5_UNORM
		{ "BC5_SNORM", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC5_SNORM
		{ "B5G6R5_UNORM", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_B5G6R5_UNORM
		{ "B5G5R5A1_UNORM", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_B5G5R5A1_UNORM
		{ "B8G8R8A8_UNORM", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_B8G8R8A8_UNORM
		{ "B8G8R8X8_UNORM", 4, 8, 8, 8, 0, false }, // TEX_FORMAT_B8G8R8X8_UNORM
		{ "R10G10B10_XR_BIAS_A2_UNORM", 4, 10, 10, 10, 2, false }, // TEX_FORMAT_R10G10B10_XR_BIAS_A2_UNORM
		{ "B8G8R8A8_TYPELESS", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_B8G8R8A8_TYPELESS
		{ "B8G8R8A8_UNORM_SRGB", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_B8G8R8A8_UNORM_SRGB
		{ "B8G8R8X8_TYPELESS", 4, 8, 8, 8, 0, false }, // TEX_FORMAT_B8G8R8X8_TYPELESS
		{ "B8G8R8X8_UNORM_SRGB", 4, 8, 8, 8, 0, false }, // TEX_FORMAT_B8G8R8X8_UNORM_SRGB
		{ "BC6H_TYPELESS", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC6H_TYPELESS
		{ "BC6H_UF16", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC6H_UF16
		{ "BC6H_SF16", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC6H_SF16
		{ "BC7_TYPELESS", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC7_TYPELESS
		{ "BC7_UNORM", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC7_UNORM
		{ "BC7_UNORM_SRGB", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC7_UNORM_SRGB
		{ "AYUV", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_AYUV
		{ "Y410", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_Y410
		{ "Y416", 8, 0, 0, 0, 0, false }, // TEX_FORMAT_Y416
		{ "NV12", 2, 0, 0, 0, 0, false }, // TEX_FORMAT_NV12
		{ "P010", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_P010
		{ "P016", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_P016
		{ "420_OPAQUE", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_420_OPAQUE
		{ "YUY2", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_YUY2
		{ "Y210", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_Y210
		{ "Y216", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_Y216
		{ "NV11", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_NV11
		{ "AI44", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_AI44
		{ "IA44", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_IA44
		{ "P8", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_P8
		{ "A8P8", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_A8P8
		{ "B4G4R4A4_UNORM", 2, 4, 4, 4, 4, false }, // TEX_FORMAT_B4G4R4A4_UNORM
		{ "P208", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_P208
		{ "V208", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_V208
		{ "V408", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_V408
	};

	const TexFormatInfo_t& TexFormatInfo( TexFormat format )
	{
		return g_TexFormatInfo[format];
	}
}