#include "VertexShader.h"

#include "SlidyAssert.h"
#include <d3dcompiler.h>
#include "COMException.h"

VertexShader::VertexShader( ID3D11Device* pDevice,  const std::wstring& filename, const D3D11_INPUT_ELEMENT_DESC* pInputElementsDesc, UINT elements )
{
	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorMessage;
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBuffer;

	if( FAILED( hr = D3DCompileFromFile( filename.c_str(), NULL, NULL, "main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage ) ) )
	{
		std::string filename_converted( filename.begin(), filename.end() );
		if( errorMessage )
		{
			std::string error = (char*)errorMessage->GetBufferPointer();
			THROW_COM_ERROR( hr, "Failed to compile vertex shader file '" + filename_converted + "'\n" + error );
		}
		else
		{
			THROW_COM_ERROR( hr, "Failed to compile vertex shader file '" + filename_converted + "'" );
		}
	}

	COM_ERROR_IF_FAILED( pDevice->CreateVertexShader( vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader ) );
	COM_ERROR_IF_FAILED( pDevice->CreateInputLayout( pInputElementsDesc, elements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout ) );
}

VertexShader::~VertexShader()
{
	for( auto pSamplerState : m_pSamplerStates )
	{
		pSamplerState->Release();
	}
}

void VertexShader::Bind( ID3D11DeviceContext* pDeviceContext )
{
	pDeviceContext->IASetInputLayout( m_pInputLayout.Get() );
	pDeviceContext->VSSetShader( m_pVertexShader.Get(), NULL, 0 );
	pDeviceContext->VSSetSamplers( 0, (UINT)m_pSamplerStates.size(), m_pSamplerStates.data() );
	for( UINT i = 0; i < m_ConstantBuffers.size(); i++ )
	{
		pDeviceContext->VSSetConstantBuffers( i, 1, m_ConstantBuffers[i].GetAddressOf() );
	}
}

void VertexShader::AddSampler( ID3D11Device* pDevice, const D3D11_SAMPLER_DESC* pSamplerDesc )
{
	ID3D11SamplerState* pSamplerState;
	COM_ERROR_IF_FAILED( pDevice->CreateSamplerState( pSamplerDesc, &pSamplerState ) );
	m_pSamplerStates.emplace_back( pSamplerState );

	ASSERT( m_pSamplerStates.size() <= D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, "Too many sampler states!" );
}

void VertexShader::SetResource( ID3D11DeviceContext* pDeviceContext, int slot, ID3D11ShaderResourceView* const pResource )
{
	pDeviceContext->VSSetShaderResources( (UINT)slot, 1, &pResource );
}
