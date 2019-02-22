#include "PCH.h"
#include "PixelShader.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include "COMException.h"
#include "RenderContext.h"
#include "MemoryStream.h"

#include "Common.h"
#include "FileWatchdog.h"

namespace Bat
{
	PixelShader::PixelShader( const std::string& filename )
		:
		m_szFilename( filename )
	{
		LoadFromFile( Bat::StringToWide( filename ), true );
		FileWatchdog::AddFileChangeListener( filename, BIND_MEM_FN( PixelShader::OnFileChanged ) );

#ifdef _DEBUG
		m_pPixelShader->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT)filename.size(), filename.c_str() );
#endif
	}

	void PixelShader::Bind()
	{
		if( IsDirty() )
		{
			while( true )
			{
				auto code = MemoryStream::FromFile( m_szFilename );
				if( code.Size() > 0 )
				{
					break;
				}
			}
			LoadFromFile( Bat::StringToWide( m_szFilename ), false );
			SetDirty( false );
		}

		auto pDeviceContext = RenderContext::GetDeviceContext();

		pDeviceContext->PSSetShader( m_pPixelShader.Get(), NULL, 0 );
		RenderContext::BindSamplers( ShaderType::PIXEL_SHADER );

		if( !m_ConstantBuffers.empty() )
		{
			std::vector<ID3D11Buffer*> buffers;
			buffers.reserve( m_ConstantBuffers.size() );
			for( const auto& buffer : m_ConstantBuffers )
			{
				buffers.emplace_back( buffer );
			}

			pDeviceContext->PSSetConstantBuffers( 0, (UINT)buffers.size(), buffers.data() );
		}
	}

	void PixelShader::SetResource( int slot, ID3D11ShaderResourceView* const pResource )
	{
		auto pDeviceContext = RenderContext::GetDeviceContext();
		pDeviceContext->PSSetShaderResources( (UINT)slot, 1, &pResource );
	}

	void PixelShader::SetResources( int startslot, ID3D11ShaderResourceView ** const pResource, size_t size )
	{
		auto pDeviceContext = RenderContext::GetDeviceContext();
		pDeviceContext->PSSetShaderResources( (UINT)startslot, (UINT)size, pResource );
	}

	void PixelShader::LoadFromFile( const std::wstring& filename, bool crash_on_error )
	{
		auto pDevice = RenderContext::GetDevice();

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
			const UINT flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR |
				D3DCOMPILE_ENABLE_STRICTNESS |
				D3DCOMPILE_DEBUG |
				D3DCOMPILE_SKIP_OPTIMIZATION;
#else
			const UINT flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS;
#endif

			if( FAILED( hr = D3DCompileFromFile( filename.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", flags, 0, &pixelShaderBuffer, &errorMessage ) ) )
			{
				const std::string filename_converted = WideToString( filename );
				if( errorMessage )
				{
					const std::string error = (char*)errorMessage->GetBufferPointer();
					if( crash_on_error )
					{
						THROW_COM_ERROR( hr, "Failed to compile pixel shader file '" + filename_converted + "'\n" + error );
					}
					else
					{
						BAT_ERROR( Bat::Trim( error ) );
						return;
					}
				}
				else
				{
					if( crash_on_error )
					{
						THROW_COM_ERROR( hr, "Failed to compile pixel shader file '" + filename_converted + "'" );
					}
					else
					{
						BAT_ERROR( "Failed to compile pixel shader file '%s'.", filename_converted );
						return;
					}
				}
			}

			COM_THROW_IF_FAILED( pDevice->CreatePixelShader( pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader ) );
		}
	}

	void PixelShader::OnFileChanged( const std::string& filename )
	{
		SetDirty( true );
	}
}