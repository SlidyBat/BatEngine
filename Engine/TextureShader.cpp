#include "TextureShader.h"
#include "TexVertex.h"
#include <fstream>

TextureShader::TextureShader( ID3D11Device * pDevice, HWND hWnd, const std::wstring & vsFilename, const std::wstring & psFilename )
{
	Microsoft::WRL::ComPtr<ID3DBlob> errorMessage;

	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBuffer;
	if( FAILED( D3DCompileFromFile( vsFilename.c_str(), NULL, NULL, "VSMain", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage ) ) )
	{
		if( errorMessage )
		{
			OutputShaderErrorMessage( errorMessage.Get(), hWnd, vsFilename );
		}
		else
		{
			throw std::runtime_error( "Failed to find vertex shader file" );
		}
	}

	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBuffer;
	if( FAILED( D3DCompileFromFile( psFilename.c_str(), NULL, NULL, "PSMain", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage ) ) )
	{
		if( errorMessage )
		{
			OutputShaderErrorMessage( errorMessage.Get(), hWnd, psFilename );
		}
		else
		{
			throw std::runtime_error( "Failed to find pixel shader file" );
		}
	}

	if( FAILED( pDevice->CreateVertexShader( vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader ) ) )
	{
		throw std::runtime_error( "Failed to create vertex shader file" );
	}
	if( FAILED( pDevice->CreatePixelShader( pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader ) ) )
	{
		throw std::runtime_error( "Failed to create pixel shader file" );
	}

	if( FAILED( pDevice->CreateInputLayout( TexVertex::InputLayout, TexVertex::Inputs, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout ) ) )
	{
		throw std::runtime_error( "Failed to create input layout" );
	}

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

	pDevice->CreateSamplerState( &samplerDesc, &m_pSamplerState );
}

bool TextureShader::Render( ID3D11DeviceContext* pDeviceContext, UINT nVertices, ID3D11ShaderResourceView* pTexture )
{
	SetShaderParameters( pDeviceContext, pTexture );
	RenderShader( pDeviceContext, nVertices );

	return true;
}

bool TextureShader::RenderIndexed( ID3D11DeviceContext* pDeviceContext, UINT nIndexes, ID3D11ShaderResourceView* pTexture )
{
	SetShaderParameters( pDeviceContext, pTexture );
	RenderShaderIndexed( pDeviceContext, nIndexes );

	return true;
}

void TextureShader::OutputShaderErrorMessage( ID3DBlob* errorMessage, HWND hWnd, const std::wstring& shaderFilename )
{
	std::ofstream out( "shader_error.txt" );

	std::string error = (char*)errorMessage->GetBufferPointer();
	out << error;

	out.close();

	throw std::runtime_error( "Error compiling shader file, check shader_error.txt for more info." );
}

void TextureShader::SetShaderParameters( ID3D11DeviceContext* pDeviceContext, ID3D11ShaderResourceView* pTexture )
{
	pDeviceContext->PSSetShaderResources( 0, 1, &pTexture );
}

void TextureShader::RenderShader( ID3D11DeviceContext* pDeviceContext, UINT nVertices )
{
	pDeviceContext->IASetInputLayout( m_pInputLayout.Get() );

	pDeviceContext->VSSetShader( m_pVertexShader.Get(), NULL, 0 );
	pDeviceContext->PSSetShader( m_pPixelShader.Get(), NULL, 0 );

	pDeviceContext->PSSetSamplers( 0, 1, &m_pSamplerState );

	pDeviceContext->Draw( nVertices, 0 );
}

void TextureShader::RenderShaderIndexed( ID3D11DeviceContext* pDeviceContext, UINT nIndexes )
{
	pDeviceContext->IASetInputLayout( m_pInputLayout.Get() );

	pDeviceContext->VSSetShader( m_pVertexShader.Get(), NULL, 0 );
	pDeviceContext->PSSetShader( m_pPixelShader.Get(), NULL, 0 );

	pDeviceContext->PSSetSamplers( 0, 1, &m_pSamplerState );

	pDeviceContext->DrawIndexed( nIndexes, 0, 0 );
}