#include "TextureShader.h"
#include "TexVertex.h"
#include <fstream>
#include "COMException.h"
#include "Graphics.h"

TextureShader::TextureShader( ID3D11Device* pDevice, const std::wstring& vsFilename, const std::wstring& psFilename )
	:
	m_VertexShader(pDevice, vsFilename, TexVertex::InputLayout, TexVertex::Inputs),
	m_PixelShader(pDevice, psFilename)
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

	m_PixelShader.AddSampler( pDevice, &samplerDesc );

	m_VertexShader.AddConstantBuffer<CB_Matrix>( pDevice );
}

void TextureShader::Render( ID3D11DeviceContext* pDeviceContext, IShaderParameters* pParameters )
{
	auto pTextureParameters = static_cast<TextureShaderParameters*>(pParameters);
	m_VertexShader.Bind( pDeviceContext );
	m_PixelShader.Bind( pDeviceContext );
	m_VertexShader.GetConstantBuffer( 0 ).SetData( pDeviceContext, pTextureParameters->GetTransformMatrix() );
	m_PixelShader.SetResource( pDeviceContext, 0, pTextureParameters->GetTextureView() );

	pDeviceContext->DrawIndexed( pTextureParameters->GetIndexCount(), 0, 0 );
}
