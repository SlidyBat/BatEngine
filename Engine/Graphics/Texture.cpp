#include "Texture.h"
#include "BatAssert.h"
#include "IGraphics.h"
#include "COMException.h"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <fstream>
#include "StringLib.h"
#include "Log.h"

namespace Bat
{
	Texture::Texture( const std::wstring& filename )
	{
		auto pDevice = g_pGfx->GetDevice();
		auto pDeviceContext = g_pGfx->GetDeviceContext();


		if( !std::ifstream( filename ) )
		{
			BAT_WARN( "Could not open texture '{}', defaulting to 'error.png'", Bat::WideToString( filename ) );
			DirectX::CreateWICTextureFromFile( pDevice, pDeviceContext, L"Assets/error.png", &m_pTexture, &m_pTextureView );
		}
		else
		{
			COM_THROW_IF_FAILED(
				DirectX::CreateWICTextureFromFile( pDevice, pDeviceContext, filename.c_str(), &m_pTexture, &m_pTextureView )
			);
		}
	}

	Texture::Texture( const uint8_t* pData, size_t size )
	{
		auto pDevice = g_pGfx->GetDevice();
		auto pDeviceContext = g_pGfx->GetDeviceContext();

		COM_THROW_IF_FAILED(
			DirectX::CreateWICTextureFromMemory( pDevice, pDeviceContext, pData, size, &m_pTexture, &m_pTextureView )
		);
	}

	Texture::Texture( const Colour* pPixels, int width, int height )
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

		ID3D11Texture2D* p2DTexture;
		COM_THROW_IF_FAILED( pDevice->CreateTexture2D( &textureDesc, NULL, &p2DTexture ) );

		m_pTexture = static_cast<ID3D11Resource*>(p2DTexture);
		pDeviceContext->UpdateSubresource( m_pTexture.Get(), 0, NULL, pPixels, width * sizeof( *pPixels ), 0 );

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		pDevice->CreateShaderResourceView( m_pTexture.Get(), &srvDesc, &m_pTextureView );

		pDeviceContext->GenerateMips( m_pTextureView.Get() );
	}

	Texture::Texture( const D3DCOLORVALUE* pPixels, int width, int height )
	{
		auto pDevice = g_pGfx->GetDevice();
		auto pDeviceContext = g_pGfx->GetDeviceContext();

		D3D11_TEXTURE2D_DESC textureDesc;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 0;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		ID3D11Texture2D* p2DTexture;
		COM_THROW_IF_FAILED( pDevice->CreateTexture2D( &textureDesc, NULL, &p2DTexture ) );

		m_pTexture = static_cast<ID3D11Resource*>(p2DTexture);
		pDeviceContext->UpdateSubresource( m_pTexture.Get(), 0, NULL, pPixels, width * sizeof( *pPixels ), 0 );

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		pDevice->CreateShaderResourceView( m_pTexture.Get(), &srvDesc, &m_pTextureView );

		pDeviceContext->GenerateMips( m_pTextureView.Get() );
	}

	Texture Texture::FromDDS( const std::wstring & filename )
	{
		auto pDevice = g_pGfx->GetDevice();
		auto pDeviceContext = g_pGfx->GetDeviceContext();

		Texture tex;
		COM_THROW_IF_FAILED(
			DirectX::CreateDDSTextureFromFile( pDevice, filename.c_str(), &tex.m_pTexture, &tex.m_pTextureView )
		);

		return tex;
	}

	ID3D11ShaderResourceView* Texture::GetTextureView() const
	{
		return m_pTextureView.Get();
	}
}