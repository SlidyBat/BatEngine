#include "ColourShader.h"
#include "Vertex.h"
#include <fstream>

bool ColourShader::Initialize( ID3D11Device* pDevice, HWND hWnd, const std::wstring& vsFilename, const std::wstring& psFilename )
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
			MessageBox( hWnd, "Failed to find vertex shader file", "Error", MB_OK );
		}

		return false;
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
			MessageBox( hWnd, "Failed to find pixel shader file", "Error", MB_OK );
		}

		return false;
	}

	if( FAILED( pDevice->CreateVertexShader( vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader ) ) )
	{
		return false;
	}
	if( FAILED( pDevice->CreatePixelShader( pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader ) ) )
	{
		return false;
	}

	if( FAILED( pDevice->CreateInputLayout( Vertex::InputLayout, Vertex::Inputs, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout ) ) )
	{
		return false;
	}

	return true;
}

bool ColourShader::Render( ID3D11DeviceContext* pDeviceContext, int nVertices )
{
	RenderShader( pDeviceContext, nVertices );

	return true;
}

void ColourShader::OutputShaderErrorMessage( ID3DBlob* errorMessage, HWND hWnd, const std::wstring& shaderFilename )
{
	std::ofstream out( "shader_error.txt" );

	std::string error = (char*)errorMessage->GetBufferPointer();
	out << error;

	out.close();

	MessageBoxW( hWnd, L"Error compiling shader file, check shader_error.txt for more info.", shaderFilename.c_str(), MB_OK );
}

void ColourShader::RenderShader( ID3D11DeviceContext* pDeviceContext, int nVertices )
{
	pDeviceContext->IASetInputLayout( m_pInputLayout.Get() );

	pDeviceContext->VSSetShader( m_pVertexShader.Get(), NULL, 0 );
	pDeviceContext->PSSetShader( m_pPixelShader.Get(), NULL, 0 );

	pDeviceContext->Draw( nVertices, 0 );
}