#include "PCH.h"
#include "Texture.h"

#include "Core/Globals.h"

namespace Bat
{
	Texture::Texture( const std::string& filename )
		:
		m_pTexture( gpu->CreateTexture( filename ) )
	{}

	Texture::Texture( const char* pData, size_t size )
		:
		m_pTexture( gpu->CreateTexture( pData, size ) )
	{}

	Texture::Texture( const void* pPixels, size_t pitch, size_t width, size_t height, TexFormat format, GPUResourceUsage usage )
		:
		m_pTexture( gpu->CreateTexture( pPixels, pitch, width, height, format, usage ) )
	{}

	Texture::operator ITexture*( )
	{
		return m_pTexture.get();
	}

	ITexture* Texture::operator->()
	{
		return m_pTexture.get();
	}

	const ITexture* Texture::operator->() const
	{
		return m_pTexture.get();
	}

	Texture Texture::FromColour( const Colour* pPixels, int width, int height, GPUResourceUsage usage )
	{
		return Texture( pPixels, width * sizeof( *pPixels ), width, height, TEX_FORMAT_B8G8R8A8_UNORM, usage );
	}

	size_t Texture::GetBPPForFormat( TexFormat fmt )
	{
		switch( fmt )
		{
			case TEX_FORMAT_R32G32B32A32_TYPELESS:
			case TEX_FORMAT_R32G32B32A32_FLOAT:
			case TEX_FORMAT_R32G32B32A32_UINT:
			case TEX_FORMAT_R32G32B32A32_SINT:
				return 16;

			case TEX_FORMAT_R32G32B32_TYPELESS:
			case TEX_FORMAT_R32G32B32_FLOAT:
			case TEX_FORMAT_R32G32B32_UINT:
			case TEX_FORMAT_R32G32B32_SINT:
				return 12;

			case TEX_FORMAT_R16G16B16A16_TYPELESS:
			case TEX_FORMAT_R16G16B16A16_FLOAT:
			case TEX_FORMAT_R16G16B16A16_UNORM:
			case TEX_FORMAT_R16G16B16A16_UINT:
			case TEX_FORMAT_R16G16B16A16_SNORM:
			case TEX_FORMAT_R16G16B16A16_SINT:
			case TEX_FORMAT_R32G32_TYPELESS:
			case TEX_FORMAT_R32G32_FLOAT:
			case TEX_FORMAT_R32G32_UINT:
			case TEX_FORMAT_R32G32_SINT:
			case TEX_FORMAT_R32G8X24_TYPELESS:
			case TEX_FORMAT_D32_FLOAT_S8X24_UINT:
			case TEX_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			case TEX_FORMAT_X32_TYPELESS_G8X24_UINT:
			case TEX_FORMAT_Y416:
			case TEX_FORMAT_Y210:
			case TEX_FORMAT_Y216:
				return 8;

			case TEX_FORMAT_R10G10B10A2_TYPELESS:
			case TEX_FORMAT_R10G10B10A2_UNORM:
			case TEX_FORMAT_R10G10B10A2_UINT:
			case TEX_FORMAT_R11G11B10_FLOAT:
			case TEX_FORMAT_R8G8B8A8_TYPELESS:
			case TEX_FORMAT_R8G8B8A8_UNORM:
			case TEX_FORMAT_R8G8B8A8_UNORM_SRGB:
			case TEX_FORMAT_R8G8B8A8_UINT:
			case TEX_FORMAT_R8G8B8A8_SNORM:
			case TEX_FORMAT_R8G8B8A8_SINT:
			case TEX_FORMAT_R16G16_TYPELESS:
			case TEX_FORMAT_R16G16_FLOAT:
			case TEX_FORMAT_R16G16_UNORM:
			case TEX_FORMAT_R16G16_UINT:
			case TEX_FORMAT_R16G16_SNORM:
			case TEX_FORMAT_R16G16_SINT:
			case TEX_FORMAT_R32_TYPELESS:
			case TEX_FORMAT_D32_FLOAT:
			case TEX_FORMAT_R32_FLOAT:
			case TEX_FORMAT_R32_UINT:
			case TEX_FORMAT_R32_SINT:
			case TEX_FORMAT_R24G8_TYPELESS:
			case TEX_FORMAT_D24_UNORM_S8_UINT:
			case TEX_FORMAT_R24_UNORM_X8_TYPELESS:
			case TEX_FORMAT_X24_TYPELESS_G8_UINT:
			case TEX_FORMAT_R9G9B9E5_SHAREDEXP:
			case TEX_FORMAT_R8G8_B8G8_UNORM:
			case TEX_FORMAT_G8R8_G8B8_UNORM:
			case TEX_FORMAT_B8G8R8A8_UNORM:
			case TEX_FORMAT_B8G8R8X8_UNORM:
			case TEX_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			case TEX_FORMAT_B8G8R8A8_TYPELESS:
			case TEX_FORMAT_B8G8R8A8_UNORM_SRGB:
			case TEX_FORMAT_B8G8R8X8_TYPELESS:
			case TEX_FORMAT_B8G8R8X8_UNORM_SRGB:
			case TEX_FORMAT_AYUV:
			case TEX_FORMAT_Y410:
			case TEX_FORMAT_YUY2:
				return 4;

			case TEX_FORMAT_P010:
			case TEX_FORMAT_P016:
				return 3;

			case TEX_FORMAT_R8G8_TYPELESS:
			case TEX_FORMAT_R8G8_UNORM:
			case TEX_FORMAT_R8G8_UINT:
			case TEX_FORMAT_R8G8_SNORM:
			case TEX_FORMAT_R8G8_SINT:
			case TEX_FORMAT_R16_TYPELESS:
			case TEX_FORMAT_R16_FLOAT:
			case TEX_FORMAT_D16_UNORM:
			case TEX_FORMAT_R16_UNORM:
			case TEX_FORMAT_R16_UINT:
			case TEX_FORMAT_R16_SNORM:
			case TEX_FORMAT_R16_SINT:
			case TEX_FORMAT_B5G6R5_UNORM:
			case TEX_FORMAT_B5G5R5A1_UNORM:
			case TEX_FORMAT_A8P8:
			case TEX_FORMAT_B4G4R4A4_UNORM:
				return 2;

			case TEX_FORMAT_R8_TYPELESS:
			case TEX_FORMAT_R8_UNORM:
			case TEX_FORMAT_R8_UINT:
			case TEX_FORMAT_R8_SNORM:
			case TEX_FORMAT_R8_SINT:
			case TEX_FORMAT_A8_UNORM:
			case TEX_FORMAT_AI44:
			case TEX_FORMAT_IA44:
			case TEX_FORMAT_P8:
				return 1;

			default:
				ASSERT( false, "Unhandled texture format '%i'", fmt );
				return 0;
		}
	}
}