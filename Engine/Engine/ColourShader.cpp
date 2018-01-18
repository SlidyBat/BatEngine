#include "ColourShader.h"
#include "Vertex.h"
#include <fstream>

ColourShader::~ColourShader()
{
	if( m_pVertexShader )
	{
		m_pVertexShader->Release();
		m_pVertexShader = nullptr;
	}

	if( m_pPixelShader )
	{
		m_pPixelShader->Release();
		m_pPixelShader = nullptr;
	}

	if( m_pInputLayout )
	{
		m_pInputLayout->Release();
		m_pInputLayout = nullptr;
	}
}

bool ColourShader::Initialize( ID3D11Device* pDevice, HWND hWnd, const std::wstring& vsFilename, const std::wstring& psFilename )
{
	ID3DBlob* errorMessage;

	ID3DBlob* vertexShaderBuffer;
	if( FAILED( D3DCompileFromFile( vsFilename.c_str(), NULL, NULL, "VSMain", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage ) ) )
	{
		if( errorMessage )
		{
			OutputShaderErrorMessage( errorMessage, hWnd, vsFilename );
		}
		else
		{
			MessageBox( hWnd, "Failed to find vertex shader file", "Error", MB_OK );
		}

		return false;
	}

	ID3DBlob* pixelShaderBuffer;
	if( FAILED( D3DCompileFromFile( psFilename.c_str(), NULL, NULL, "PSMain", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage ) ) )
	{
		if( errorMessage )
		{
			OutputShaderErrorMessage( errorMessage, hWnd, psFilename );
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

	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

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

	errorMessage->Release();
	errorMessage = nullptr;

	MessageBoxW( hWnd, L"Error compiling shader file, check shader_error.txt for more info.", shaderFilename.c_str(), MB_OK );
}

void ColourShader::RenderShader( ID3D11DeviceContext* pDeviceContext, int nVertices )
{
	pDeviceContext->IASetInputLayout( m_pInputLayout );

	pDeviceContext->VSSetShader( m_pVertexShader, NULL, 0 );
	pDeviceContext->PSSetShader( m_pPixelShader, NULL, 0 );

	pDeviceContext->Draw( nVertices, 0 );
}