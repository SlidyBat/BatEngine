#include "VertexShader.h"

#include "BatAssert.h"
#include <d3dcompiler.h>
#include "COMException.h"
#include "Graphics.h"
#include "MemoryStream.h"

namespace Bat
{
	VertexShader::VertexShader( const std::wstring& filename, const D3D11_INPUT_ELEMENT_DESC* pInputElementsDesc, UINT elements )
	{
		auto pDevice = g_pGfx->GetDevice();

		// compiled shader object
		if( filename.find( L".cso" ) != std::wstring::npos )
		{
			auto bytes = MemoryStream::FromFile( filename );
			COM_ERROR_IF_FAILED( pDevice->CreateVertexShader( bytes.Base(), bytes.Size(), NULL, &m_pVertexShader ) );
			COM_ERROR_IF_FAILED( pDevice->CreateInputLayout( pInputElementsDesc, elements, bytes.Base(), bytes.Size(), &m_pInputLayout ) );
		}
		// not compiled, lets compile ourselves
		else
		{
			HRESULT hr;
			Microsoft::WRL::ComPtr<ID3DBlob> errorMessage;
			Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBuffer;

#ifdef _DEBUG
			const UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
#else
			const UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#endif

			if( FAILED( hr = D3DCompileFromFile( filename.c_str(), NULL, NULL, "main", "vs_5_0", flags, 0, &vertexShaderBuffer, &errorMessage ) ) )
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
	}

	VertexShader::~VertexShader()
	{
		for( auto pSamplerState : m_pSamplerStates )
		{
			pSamplerState->Release();
		}
	}

	void VertexShader::Bind()
	{
		auto pDeviceContext = g_pGfx->GetDeviceContext();

		pDeviceContext->IASetInputLayout( m_pInputLayout.Get() );
		pDeviceContext->VSSetShader( m_pVertexShader.Get(), NULL, 0 );
		pDeviceContext->VSSetSamplers( 0, (UINT)m_pSamplerStates.size(), m_pSamplerStates.data() );
		for( UINT i = 0; i < m_ConstantBuffers.size(); i++ )
		{
			pDeviceContext->VSSetConstantBuffers( i, 1, m_ConstantBuffers[i].GetAddressOf() );
		}
	}

	void VertexShader::AddSampler( const D3D11_SAMPLER_DESC* pSamplerDesc )
	{
		auto pDevice = g_pGfx->GetDevice();

		ID3D11SamplerState* pSamplerState;
		COM_ERROR_IF_FAILED( pDevice->CreateSamplerState( pSamplerDesc, &pSamplerState ) );
		m_pSamplerStates.emplace_back( pSamplerState );

		ASSERT( m_pSamplerStates.size() <= D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, "Too many sampler states!" );
	}

	void VertexShader::SetResource( int slot, ID3D11ShaderResourceView* const pResource )
	{
		auto pDeviceContext = g_pGfx->GetDeviceContext();
		pDeviceContext->VSSetShaderResources( (UINT)slot, 1, &pResource );
	}
}