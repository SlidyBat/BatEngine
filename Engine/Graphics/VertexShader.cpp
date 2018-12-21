#include "PCH.h"
#include "VertexShader.h"

#include "RenderContext.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include "COMException.h"
#include "Graphics.h"
#include "MemoryStream.h"

namespace Bat
{
	void VertexShader::CreateInputLayoutDescFromVertexShaderSignature( const void* pCodeBytes, const size_t size )
	{
		// Reflect shader info
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pVertexShaderReflection;

		COM_THROW_IF_FAILED(
			D3DReflect( pCodeBytes, size, IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection )
		);
		// Get shader info
		D3D11_SHADER_DESC shaderDesc;
		pVertexShaderReflection->GetDesc( &shaderDesc );

		// Read input layout description from shader info
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
		for( UINT i = 0; i < shaderDesc.InputParameters; i++ )
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			pVertexShaderReflection->GetInputParameterDesc( i, &paramDesc );

			// fill out input element desc
			D3D11_INPUT_ELEMENT_DESC elementDesc;
			elementDesc.SemanticName = paramDesc.SemanticName;
			elementDesc.SemanticIndex = paramDesc.SemanticIndex;
			elementDesc.InputSlot = i;
			elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;

			auto attribute = AttributeInfo::SemanticToAttribute( paramDesc.SemanticName );
			ASSERT( attribute != VertexAttribute::Invalid, "Unknown semantic type" );
			m_bUsesAttribute[(int)attribute] = true;

			// determine DXGI format
			if( paramDesc.Mask == 1 )
			{
				if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32_UINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32_SINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			else if( paramDesc.Mask <= 3 )
			{
				if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			}
			else if( paramDesc.Mask <= 7 )
			{
				if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if( paramDesc.Mask <= 15 )
			{
				if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			//save element desc
			inputLayoutDesc.push_back( elementDesc );
		}

		RenderContext::GetDevice()->CreateInputLayout( inputLayoutDesc.data(), (UINT)inputLayoutDesc.size(), pCodeBytes, size, &m_pInputLayout );
	}

	VertexShader::VertexShader( const std::wstring& filename )
	{
		for( int i = 0; i < (int)VertexAttribute::TotalAttributes; i++ )
		{
			m_bUsesAttribute[i] = false;
		}

		auto pDevice = RenderContext::GetDevice();

		// compiled shader object
		if( Bat::GetFileExtension( filename ) == L"cso" )
		{
			auto bytes = MemoryStream::FromFile( filename );
			CreateInputLayoutDescFromVertexShaderSignature( bytes.Base(), bytes.Size() );

			COM_THROW_IF_FAILED( pDevice->CreateVertexShader( bytes.Base(), bytes.Size(), NULL, &m_pVertexShader ) );
			CreateInputLayoutDescFromVertexShaderSignature( bytes.Base(), bytes.Size() );
		}
		// not compiled, lets compile ourselves
		else
		{
			HRESULT hr;
			Microsoft::WRL::ComPtr<ID3DBlob> errorMessage;
			Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBuffer;

#ifdef _DEBUG
			const UINT flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
#else
			const UINT flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS;
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

			COM_THROW_IF_FAILED( pDevice->CreateVertexShader( vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader ) );
			CreateInputLayoutDescFromVertexShaderSignature( vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize() );
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
		auto pDeviceContext = RenderContext::GetDeviceContext();

		pDeviceContext->IASetInputLayout( m_pInputLayout.Get() );
		pDeviceContext->VSSetShader( m_pVertexShader.Get(), NULL, 0 );
		pDeviceContext->VSSetSamplers( 0, (UINT)m_pSamplerStates.size(), m_pSamplerStates.data() );

		std::vector<ID3D11Buffer*> buffers;
		buffers.reserve( m_ConstantBuffers.size() );
		for( const auto& buffer : m_ConstantBuffers )
		{
			buffers.emplace_back( buffer );
		}

		pDeviceContext->VSSetConstantBuffers( 0, (UINT)buffers.size(), buffers.data() );
	}

	void VertexShader::AddSampler( const D3D11_SAMPLER_DESC* pSamplerDesc )
	{
		auto pDevice = RenderContext::GetDevice();

		ID3D11SamplerState* pSamplerState;
		COM_THROW_IF_FAILED( pDevice->CreateSamplerState( pSamplerDesc, &pSamplerState ) );
		m_pSamplerStates.emplace_back( pSamplerState );

		ASSERT( m_pSamplerStates.size() <= D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, "Too many sampler states!" );
	}

	void VertexShader::SetResource( const int slot, ID3D11ShaderResourceView* const pResource )
	{
		auto pDeviceContext = RenderContext::GetDeviceContext();
		pDeviceContext->VSSetShaderResources( (UINT)slot, 1, &pResource );
	}
}