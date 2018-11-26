#pragma once

#include <d3d11.h>
#include <string>
#include <vector>
#include <wrl.h>

#include "ConstantBuffer.h"
#include "VertexTypes.h"

namespace Bat
{
	class VertexShader
	{
	public:
		VertexShader( const std::wstring& filename );
		~VertexShader();

		void Bind();
		void AddSampler( const D3D11_SAMPLER_DESC* pSamplerDesc );
		void SetResource( const int slot, ID3D11ShaderResourceView* const pResource );

		template <typename T>
		void AddConstantBuffer()
		{
			m_ConstantBuffers.emplace_back( sizeof( T ) );
		}
		template <typename T>
		void AddConstantBuffer( T* pData )
		{
			m_ConstantBuffers.emplace_back( pData, sizeof( T ) );
		}

		ConstantBuffer& GetConstantBuffer( const int slot )
		{
			return m_ConstantBuffers[slot];
		}

		bool RequiresVertexAttribute( const VertexAttribute attribute )
		{
			return m_bUsesAttribute[(int)attribute];
		}
	private:
		void CreateInputLayoutDescFromVertexShaderSignature( const void* pCodeBytes, const size_t size );
	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_pVertexShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_pInputLayout;
		bool m_bUsesAttribute[(int)VertexAttribute::TotalAttributes];
		std::vector<ID3D11SamplerState*> m_pSamplerStates;
		std::vector<ConstantBuffer> m_ConstantBuffers;
	};
}