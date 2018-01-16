#include "ColourShader.h"
#include <string>

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

bool ColourShader::Initialize( ID3D11Device* pDevice, HWND hWnd )
{
	return InitializeShader( pDevice, hWnd, L"../Engine/Colour.vs", L"../Engine/Colour.ps" );
}

bool ColourShader::Render( ID3D11DeviceContext* pDeviceContext, int nIndexes )
{
	RenderShader( pDeviceContext, nIndexes );

	return true;
}

bool ColourShader::InitializeShader( ID3D11Device* pDevice, HWND hWnd, const std::wstring& vsFilename, const std::wstring& psFilename )
{
	ID3DBlob* errorMessage;

	ID3DBlob* vertexShaderBuffer;
	if( FAILED( D3DCompileFromFile( vsFilename.c_str(), NULL, NULL, "ColourVertexShader", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage ) ) )
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
	if( FAILED( D3DCompileFromFile( psFilename.c_str(), NULL, NULL, "ColourPixelShader", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage ) ) )
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

	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];

	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;
	polygonLayout[0].AlignedByteOffset = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

	int nElements = sizeof( polygonLayout ) / sizeof( polygonLayout[0] );

	if( FAILED( pDevice->CreateInputLayout( polygonLayout, nElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout ) ) )
	{
		return false;
	}

	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	return true;
}

void ColourShader::OutputShaderErrorMessage( ID3D10Blob* errorMessage, HWND hWnd, const std::wstring& shaderFilename )
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