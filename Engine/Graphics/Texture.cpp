#include "Texture.h"
#include "BatAssert.h"

#define FULL_WINTARD
#include "BatWinAPI.h"

#include <algorithm>
namespace Gdiplus
{
	using std::min;
	using std::max;
}
#include <gdiplus.h>
#include "Graphics.h"

namespace Bat
{
	Texture::Texture( const std::wstring& filename )
	{
		auto pDevice = g_pGfx->GetDevice();
		auto pDeviceContext = g_pGfx->GetDeviceContext();

		Gdiplus::Bitmap bmp( filename.c_str() );
		ASSERT( bmp.GetPixelFormat() == PixelFormat32bppARGB, "Unsupported BMP format" );

		Colour* pPixels = new Colour[bmp.GetWidth()*bmp.GetHeight()];
		for( size_t y = 0; y < bmp.GetHeight(); y++ )
		{
			for( size_t x = 0; x < bmp.GetWidth(); x++ )
			{
				Gdiplus::Color c;
				bmp.GetPixel( (int)x, (int)y, &c );
				pPixels[x + y * bmp.GetWidth()].SetValue( c.GetValue() );
			}
		}

		D3D11_TEXTURE2D_DESC textureDesc;
		textureDesc.Width = bmp.GetWidth();
		textureDesc.Height = bmp.GetHeight();
		textureDesc.MipLevels = 0;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		if( FAILED( pDevice->CreateTexture2D( &textureDesc, NULL, &m_pTexture ) ) )
		{
			throw std::runtime_error( "Failed to create texture" );
		}
		pDeviceContext->UpdateSubresource( m_pTexture.Get(), 0, NULL, pPixels, bmp.GetWidth() * sizeof( Colour ), 0 );

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		pDevice->CreateShaderResourceView( m_pTexture.Get(), &srvDesc, &m_pTextureView );

		pDeviceContext->GenerateMips( m_pTextureView.Get() );

		delete[] pPixels;
	}

	Texture::Texture( const Colour * pPixels, int width, int height )
	{
		auto pDevice = g_pGfx->GetDevice();
		auto pDeviceContext = g_pGfx->GetDeviceContext();

		D3D11_TEXTURE2D_DESC textureDesc;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 0;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		if( FAILED( pDevice->CreateTexture2D( &textureDesc, NULL, &m_pTexture ) ) )
		{
			throw std::runtime_error( "Failed to create texture" );
		}
		pDeviceContext->UpdateSubresource( m_pTexture.Get(), 0, NULL, pPixels, width * sizeof( Colour ), 0 );

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		pDevice->CreateShaderResourceView( m_pTexture.Get(), &srvDesc, &m_pTextureView );

		pDeviceContext->GenerateMips( m_pTextureView.Get() );
	}

	ID3D11ShaderResourceView* Texture::GetTextureView() const
	{
		return m_pTextureView.Get();
	}
}