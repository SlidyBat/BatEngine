#include "PCH.h"
#include "Texture.h"

#include <d3d11.h>
#include "RenderContext.h"
#include "COMException.h"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

namespace Bat
{
	Texture::Texture( const std::wstring& filename )
	{
		auto pDevice = RenderContext::GetDevice();
		auto pDeviceContext = RenderContext::GetDeviceContext();

		if( !std::ifstream( filename ) )
		{
			BAT_WARN( "Could not open texture '%s', defaulting to 'error.png'", Bat::WideToString( filename ) );
			COM_THROW_IF_FAILED(
				DirectX::CreateWICTextureFromFile( pDevice, L"Assets/error.png", &m_pTexture, &m_pTextureView )
			);
		}
		else if( Bat::GetFileExtension( filename ) != L"dds" )
		{
			COM_THROW_IF_FAILED(
				DirectX::CreateWICTextureFromFile( pDevice, nullptr, filename.c_str(), &m_pTexture, &m_pTextureView )
			);
		}
		else
		{
			COM_THROW_IF_FAILED(
				DirectX::CreateDDSTextureFromFile( pDevice, filename.c_str(), &m_pTexture, &m_pTextureView )
			);
		}

		// get width/height
		Microsoft::WRL::ComPtr<ID3D11Resource> pResource;
		m_pTextureView->GetResource( &pResource );
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture2D;
		pResource.As( &pTexture2D );
		D3D11_TEXTURE2D_DESC desc;
		pTexture2D->GetDesc( &desc );
		m_iWidth = desc.Width;
		m_iHeight = desc.Height;

#ifdef _DEBUG
		m_pTexture->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT)filename.size(), filename.c_str() );
#endif
	}

	Texture::Texture( const uint8_t* pData, size_t size )
	{
		auto pDevice = RenderContext::GetDevice();
		auto pDeviceContext = RenderContext::GetDeviceContext();

		COM_THROW_IF_FAILED(
			DirectX::CreateWICTextureFromMemory( pDevice, nullptr, pData, size, &m_pTexture, &m_pTextureView )
		);

		// get width/height
		Microsoft::WRL::ComPtr<ID3D11Resource> pResource;
		m_pTextureView->GetResource( &pResource );
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture2D;
		pResource.As( &pTexture2D );
		D3D11_TEXTURE2D_DESC desc;
		pTexture2D->GetDesc( &desc );
		m_iWidth = desc.Width;
		m_iHeight = desc.Height;
	}

	Texture::Texture( const void* pPixels, size_t pitch, int width, int height, DXGI_FORMAT format, D3D11_USAGE usage )
	{
		auto pDevice = RenderContext::GetDevice();

		UINT flags = 0;
		if( usage == D3D11_USAGE_DYNAMIC )
		{
			flags = D3D11_CPU_ACCESS_WRITE;
		}

		CD3D11_TEXTURE2D_DESC textureDesc( format, width, height, 1, 1, D3D11_BIND_SHADER_RESOURCE, usage, flags );
		ID3D11Texture2D* p2DTexture;

		if( pPixels )
		{
			D3D11_SUBRESOURCE_DATA initialData{};
			initialData.pSysMem = pPixels;
			initialData.SysMemPitch = (UINT)pitch;
			COM_THROW_IF_FAILED( pDevice->CreateTexture2D( &textureDesc, &initialData, &p2DTexture ) );
		}
		else
		{
			COM_THROW_IF_FAILED( pDevice->CreateTexture2D( &textureDesc, nullptr, &p2DTexture ) );
		}

		m_pTexture = static_cast<ID3D11Resource*>( p2DTexture );

		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc( D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format );
		COM_THROW_IF_FAILED(
			pDevice->CreateShaderResourceView( m_pTexture.Get(), &srvDesc, &m_pTextureView )
		);

		m_iWidth = width;
		m_iHeight = height;
	}

	Texture Texture::FromDDS( const std::wstring& filename )
	{
		auto pDevice = RenderContext::GetDevice();

		Texture tex;
		COM_THROW_IF_FAILED(
			DirectX::CreateDDSTextureFromFile( pDevice, filename.c_str(), &tex.m_pTexture, &tex.m_pTextureView )
		);

		return tex;
	}

	Texture Texture::FromColour( const Colour* pPixels, int width, int height, D3D11_USAGE usage )
	{
		return Texture( pPixels, width * sizeof( *pPixels ), width, height, DXGI_FORMAT_B8G8R8A8_UNORM, usage );
	}

	Texture Texture::FromD3DCOLORVALUE( const D3DCOLORVALUE* pPixels, int width, int height, D3D11_USAGE usage )
	{
		return Texture( pPixels, width * sizeof( *pPixels ), width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, usage );
	}

	void Texture::UpdatePixels( const void* pPixels, size_t pitch )
	{
		auto pDeviceContext = RenderContext::GetDeviceContext();

		D3D11_MAPPED_SUBRESOURCE mapped_tex;
		COM_THROW_IF_FAILED( pDeviceContext->Map( m_pTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_tex ) );

		const char* pSrcBytes = reinterpret_cast<const char*>( pPixels );
		char* pDstBytes = reinterpret_cast<char*>( mapped_tex.pData );
		const UINT min_pitch = std::min( (UINT)pitch, mapped_tex.RowPitch );

		// perform the copy line-by-line
		for( size_t y = 0; y < GetHeight(); y++ )
		{
			memcpy( &pDstBytes[ y * mapped_tex.RowPitch ], &pSrcBytes[y * pitch], min_pitch );
		}

		pDeviceContext->Unmap( m_pTexture.Get(), 0 );
	}

	ID3D11ShaderResourceView* Texture::GetTextureView() const
	{
		return m_pTextureView.Get();
	}

	size_t Texture::GetBPPForFormat( DXGI_FORMAT fmt )
	{
		switch( static_cast<int>( fmt ) )
		{
			case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
			case DXGI_FORMAT_R32G32B32A32_UINT:
			case DXGI_FORMAT_R32G32B32A32_SINT:
				return 16;

			case DXGI_FORMAT_R32G32B32_TYPELESS:
			case DXGI_FORMAT_R32G32B32_FLOAT:
			case DXGI_FORMAT_R32G32B32_UINT:
			case DXGI_FORMAT_R32G32B32_SINT:
				return 12;

			case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_UNORM:
			case DXGI_FORMAT_R16G16B16A16_UINT:
			case DXGI_FORMAT_R16G16B16A16_SNORM:
			case DXGI_FORMAT_R16G16B16A16_SINT:
			case DXGI_FORMAT_R32G32_TYPELESS:
			case DXGI_FORMAT_R32G32_FLOAT:
			case DXGI_FORMAT_R32G32_UINT:
			case DXGI_FORMAT_R32G32_SINT:
			case DXGI_FORMAT_R32G8X24_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			case DXGI_FORMAT_Y416:
			case DXGI_FORMAT_Y210:
			case DXGI_FORMAT_Y216:
				return 8;

			case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			case DXGI_FORMAT_R10G10B10A2_UNORM:
			case DXGI_FORMAT_R10G10B10A2_UINT:
			case DXGI_FORMAT_R11G11B10_FLOAT:
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_R8G8B8A8_UINT:
			case DXGI_FORMAT_R8G8B8A8_SNORM:
			case DXGI_FORMAT_R8G8B8A8_SINT:
			case DXGI_FORMAT_R16G16_TYPELESS:
			case DXGI_FORMAT_R16G16_FLOAT:
			case DXGI_FORMAT_R16G16_UNORM:
			case DXGI_FORMAT_R16G16_UINT:
			case DXGI_FORMAT_R16G16_SNORM:
			case DXGI_FORMAT_R16G16_SINT:
			case DXGI_FORMAT_R32_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT:
			case DXGI_FORMAT_R32_FLOAT:
			case DXGI_FORMAT_R32_UINT:
			case DXGI_FORMAT_R32_SINT:
			case DXGI_FORMAT_R24G8_TYPELESS:
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
			case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			case DXGI_FORMAT_R8G8_B8G8_UNORM:
			case DXGI_FORMAT_G8R8_G8B8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8X8_UNORM:
			case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			case DXGI_FORMAT_AYUV:
			case DXGI_FORMAT_Y410:
			case DXGI_FORMAT_YUY2:
				return 4;

			case DXGI_FORMAT_P010:
			case DXGI_FORMAT_P016:
				return 3;

			case DXGI_FORMAT_R8G8_TYPELESS:
			case DXGI_FORMAT_R8G8_UNORM:
			case DXGI_FORMAT_R8G8_UINT:
			case DXGI_FORMAT_R8G8_SNORM:
			case DXGI_FORMAT_R8G8_SINT:
			case DXGI_FORMAT_R16_TYPELESS:
			case DXGI_FORMAT_R16_FLOAT:
			case DXGI_FORMAT_D16_UNORM:
			case DXGI_FORMAT_R16_UNORM:
			case DXGI_FORMAT_R16_UINT:
			case DXGI_FORMAT_R16_SNORM:
			case DXGI_FORMAT_R16_SINT:
			case DXGI_FORMAT_B5G6R5_UNORM:
			case DXGI_FORMAT_B5G5R5A1_UNORM:
			case DXGI_FORMAT_A8P8:
			case DXGI_FORMAT_B4G4R4A4_UNORM:
				return 2;

			case DXGI_FORMAT_R8_TYPELESS:
			case DXGI_FORMAT_R8_UNORM:
			case DXGI_FORMAT_R8_UINT:
			case DXGI_FORMAT_R8_SNORM:
			case DXGI_FORMAT_R8_SINT:
			case DXGI_FORMAT_A8_UNORM:
			case DXGI_FORMAT_AI44:
			case DXGI_FORMAT_IA44:
			case DXGI_FORMAT_P8:
				return 1;

			default:
				ASSERT( false, "Unhandled texture format" );
				return 0;
		}
	}
}