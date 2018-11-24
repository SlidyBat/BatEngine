#include "PixelShader.h"

#include "BatAssert.h"
#include <d3dcompiler.h>
#include "COMException.h"
#include "Graphics.h"
#include "MemoryStream.h"

namespace Bat
{
	PixelShader::PixelShader( const std::wstring& filename )
	{
		auto pDevice = g_pGfx->GetDevice();

		// compiled shader object
		if( filename.find( L".cso" ) != std::wstring::npos )
		{
			auto bytes = MemoryStream::FromFile( filename );
			COM_ERROR_IF_FAILED( pDevice->CreatePixelShader( bytes.Base(), bytes.Size(), NULL, &m_pPixelShader ) );
		}
		// not compiled, lets compile ourselves
		else
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
	}

	PixelShader::~PixelShader()
	{
		for( auto pSamplerState : m_pSamplerStates )
		{
			pSamplerState->Release();
		}
	}

	void PixelShader::Bind()
	{
		auto pDeviceContext = g_pGfx->GetDeviceContext();

		pDeviceContext->PSSetShader( m_pPixelShader.Get(), NULL, 0 );
		pDeviceContext->PSSetSamplers( 0, (UINT)m_pSamplerStates.size(), m_pSamplerStates.data() );
		for( UINT i = 0; i < m_ConstantBuffers.size(); i++ )
		{
			pDeviceContext->PSSetConstantBuffers( i, 1, m_ConstantBuffers[i].GetAddressOf() );
		}
	}

	void PixelShader::AddSampler( const D3D11_SAMPLER_DESC* pSamplerDesc )
	{
		auto pDevice = g_pGfx->GetDevice();

		ID3D11SamplerState* pSamplerState;
		COM_ERROR_IF_FAILED( pDevice->CreateSamplerState( pSamplerDesc, &pSamplerState ) );
		m_pSamplerStates.emplace_back( pSamplerState );

		ASSERT( m_pSamplerStates.size() < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, "Too many samplers!" );
	}

	void PixelShader::SetResource( int slot, ID3D11ShaderResourceView* const pResource )
	{
		auto pDeviceContext = g_pGfx->GetDeviceContext();
		pDeviceContext->PSSetShaderResources( (UINT)slot, 1, &pResource );
	}
}