#include "PCH.h"
#include "RenderTexture.h"

#include "IGraphics.h"
#include "COMException.h"
#include <d3d11.h>

namespace Bat
{
	RenderTexture::RenderTexture( int width, int height )
	{
		Resize( width, height );
	}

	void RenderTexture::Bind()
	{
		g_pGfx->GetDeviceContext()->OMSetRenderTargets( 1, m_pRenderTargetView.GetAddressOf(), g_pGfx->GetDepthStencilView() );
	}

	void RenderTexture::Clear( const float red, const float green, const float blue, const float alpha )
	{
		float colour[4] = { red, green, blue, alpha };

		g_pGfx->GetDeviceContext()->ClearRenderTargetView( m_pRenderTargetView.Get(), colour );
		g_pGfx->GetDeviceContext()->ClearDepthStencilView( g_pGfx->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0 );
	}

	void RenderTexture::Resize( int width, int height )
	{
		m_iWidth = width;
		m_iHeight = height;

		const auto pDevice = g_pGfx->GetDevice();

		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		COM_THROW_IF_FAILED( pDevice->CreateTexture2D( &textureDesc, NULL, &m_pRenderTargetTexture ) );

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
		COM_THROW_IF_FAILED( pDevice->CreateRenderTargetView( m_pRenderTargetTexture.Get(), &renderTargetViewDesc, &m_pRenderTargetView ) );

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		COM_THROW_IF_FAILED( pDevice->CreateShaderResourceView( m_pRenderTargetTexture.Get(), &shaderResourceViewDesc, &m_pShaderResourceView ) );
	}
}