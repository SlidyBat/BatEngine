#include "PixelShader.h"

#include <cassert>
#include <d3dcompiler.h>
#include "COMException.h"

PixelShader::PixelShader( ID3D11Device* pDevice, const std::wstring& filename )
{
	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorMessage;

	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBuffer;
	if( FAILED( hr = D3DCompileFromFile( filename.c_str(), NULL, NULL, "main", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage ) ) )
	{
		std::string filename_converted( filename.begin(), filename.end() );
		if( errorMessage )
		{
			std::string error = (char*)errorMessage->GetBufferPointer();
			THROW_COM_ERROR( hr, "Failed to compile pixel shader file '" + filename_converted + "'\n" + error );
		}
		else
		{
			THROW_COM_ERROR( hr, "Failed to compile pixel shader file '" + filename_converted + "'" );
		}
	}

	COM_ERROR_IF_FAILED( pDevice->CreatePixelShader( pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader ) );
}

PixelShader::~PixelShader()
{
	for( auto pSamplerState : m_pSamplerStates )
	{
		pSamplerState->Release();
	}
}

void PixelShader::Bind( ID3D11DeviceContext* pDeviceContext )
{
	pDeviceContext->PSSetShader( m_pPixelShader.Get(), NULL, 0 );
	pDeviceContext->PSSetSamplers( 0, (UINT)m_pSamplerStates.size(), m_pSamplerStates.data() );
	for( UINT i = 0; i < m_ConstantBuffers.size(); i++ )
	{
		pDeviceContext->PSSetConstantBuffers( i, 1, m_ConstantBuffers[i].GetAddressOf() );
	}
}

void PixelShader::AddSampler( ID3D11Device* pDevice, const D3D11_SAMPLER_DESC* pSamplerDesc )
{
	ID3D11SamplerState* pSamplerState;
	COM_ERROR_IF_FAILED( pDevice->CreateSamplerState( pSamplerDesc, &pSamplerState ) );
	m_pSamplerStates.emplace_back( pSamplerState );

	assert( m_pSamplerStates.size() < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT );
}

void PixelShader::SetResource( ID3D11DeviceContext* pDeviceContext, int slot, ID3D11ShaderResourceView* const pResource )
{
	pDeviceContext->PSSetShaderResources( (UINT)slot, 1, &pResource );
}
