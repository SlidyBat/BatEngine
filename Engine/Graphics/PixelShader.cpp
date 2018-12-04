#include "PixelShader.h"

#include "BatAssert.h"
#include <d3dcompiler.h>
#include "COMException.h"
#include "Graphics.h"
#include "MemoryStream.h"
#include "StringLib.h"

namespace Bat
{
	PixelShader::PixelShader( const std::wstring& filename )
	{
		auto pDevice = g_pGfx->GetDevice();

		// compiled shader object
		if( Bat::GetFileExtension( filename ) == L"cso" )
		{
			auto bytes = MemoryStream::FromFile( filename );
			COM_THROW_IF_FAILED( pDevice->CreatePixelShader( bytes.Base(), bytes.Size(), NULL, &m_pPixelShader ) );
		}
		// not compiled, lets compile ourselves
		else
		{
			HRESULT hr;
			Microsoft::WRL::ComPtr<ID3DBlob> errorMessage;
			Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBuffer;

#ifdef _DEBUG
			const UINT flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
#else
			const UINT flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS;
#endif

			if( FAILED( hr = D3DCompileFromFile( filename.c_str(), NULL, NULL, "main", "ps_5_0", flags, 0, &pixelShaderBuffer, &errorMessage ) ) )
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

			COM_THROW_IF_FAILED( pDevice->CreatePixelShader( pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader ) );
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

		std::vector<ID3D11Buffer*> buffers;
		buffers.reserve( m_ConstantBuffers.size() );
		for( const auto& buffer : m_ConstantBuffers )
		{
			buffers.emplace_back( buffer );
		}

		pDeviceContext->PSSetConstantBuffers( 0, (UINT)buffers.size(), buffers.data() );
	}

	void PixelShader::AddSampler( const D3D11_SAMPLER_DESC* pSamplerDesc )
	{
		auto pDevice = g_pGfx->GetDevice();

		ID3D11SamplerState* pSamplerState;
		COM_THROW_IF_FAILED( pDevice->CreateSamplerState( pSamplerDesc, &pSamplerState ) );
		m_pSamplerStates.emplace_back( pSamplerState );

		ASSERT( m_pSamplerStates.size() < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, "Too many samplers!" );
	}

	void PixelShader::SetResource( int slot, ID3D11ShaderResourceView* const pResource )
	{
		auto pDeviceContext = g_pGfx->GetDeviceContext();
		pDeviceContext->PSSetShaderResources( (UINT)slot, 1, &pResource );
	}

	void PixelShader::SetResources( int startslot, ID3D11ShaderResourceView ** const pResource, size_t size )
	{
		auto pDeviceContext = g_pGfx->GetDeviceContext();
		pDeviceContext->PSSetShaderResources( (UINT)startslot, (UINT)size, pResource );
	}
}