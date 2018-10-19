#include "ColourShader.h"
#include "Vertex.h"
#include <fstream>


ColourShader::ColourShader( ID3D11Device* pDevice, HWND hWnd, const std::wstring& vsFilename, const std::wstring& psFilename )
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

	if( FAILED( pDevice->CreateInputLayout( Vertex::InputLayout, Vertex::Inputs, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout ) ) )
	{
		throw std::runtime_error( "Failed to create input layout" );
	}
}

bool ColourShader::Render( ID3D11DeviceContext* pDeviceContext, size_t nVertices )
{
	RenderShader( pDeviceContext, nVertices );

	return true;
}

bool ColourShader::RenderIndexed( ID3D11DeviceContext* pDeviceContext, size_t nIndexes )
{
	RenderShader( pDeviceContext, nIndexes );

	return true;
}

void ColourShader::OutputShaderErrorMessage( ID3DBlob* errorMessage, HWND hWnd, const std::wstring& shaderFilename )
{
	std::ofstream out( "shader_error.txt" );

	std::string error = (char*)errorMessage->GetBufferPointer();
	out << error;

	out.close();

	throw std::runtime_error( "Error compiling shader file, check shader_error.txt for more info" );
}

void ColourShader::RenderShader( ID3D11DeviceContext* pDeviceContext, size_t nVertices )
{
	pDeviceContext->IASetInputLayout( m_pInputLayout.Get() );

	pDeviceContext->VSSetShader( m_pVertexShader.Get(), NULL, 0 );
	pDeviceContext->PSSetShader( m_pPixelShader.Get(), NULL, 0 );

	pDeviceContext->Draw( (UINT)nVertices, 0 );
}

void ColourShader::RenderShaderIndexed( ID3D11DeviceContext* pDeviceContext, size_t nIndexes )
{
	pDeviceContext->IASetInputLayout( m_pInputLayout.Get() );

	pDeviceContext->VSSetShader( m_pVertexShader.Get(), NULL, 0 );
	pDeviceContext->PSSetShader( m_pPixelShader.Get(), NULL, 0 );

	pDeviceContext->DrawIndexed( (UINT)nIndexes, 0, 0 );
}