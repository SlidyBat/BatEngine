#include "TextureShader.h"
#include "VertexTypes.h"
#include <fstream>
#include "COMException.h"
#include "Graphics.h"

namespace Bat
{
	TextureShader::TextureShader( const std::wstring& vsFilename, const std::wstring& psFilename )
		:
		m_VertexShader( vsFilename, TexVertex::InputLayout, TexVertex::Inputs ),
		m_PixelShader( psFilename )
	{
		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		m_PixelShader.AddSampler( &samplerDesc );

		m_VertexShader.AddConstantBuffer<CB_Matrix>();
	}

	void TextureShader::BindParameters( IShaderParameters* pParameters )
	{
		auto pTextureParameters = static_cast<TextureShaderParameters*>(pParameters);
		m_VertexShader.Bind();
		m_PixelShader.Bind();
		m_VertexShader.GetConstantBuffer( 0 ).SetData( pTextureParameters->GetTransformMatrix() );
		m_PixelShader.SetResource( 0, pTextureParameters->GetTextureView() );
	}

	void TextureShader::Render( UINT vertexcount )
	{
		auto pDeviceContext = g_pGfx->GetDeviceContext();
		pDeviceContext->Draw( vertexcount, 0 );
	}

	void TextureShader::RenderIndexed( UINT indexcount )
	{
		auto pDeviceContext = g_pGfx->GetDeviceContext();
		pDeviceContext->DrawIndexed( indexcount, 0, 0 );
	}
}