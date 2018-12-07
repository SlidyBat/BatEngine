#pragma once

#include "PCH.h"

#include <wrl.h>
#include "ConstantBuffer.h"

struct ID3D11PixelShader;

namespace Bat
{
	class PixelShader
	{
	public:
		PixelShader( const std::wstring& filename );
		~PixelShader();

		void Bind();
		void AddSampler( const struct D3D11_SAMPLER_DESC* pSamplerDesc );
		void SetResource( int slot, struct ID3D11ShaderResourceView* const pResource );
		void SetResources( int startslot, struct ID3D11ShaderResourceView** const pResource, size_t size );

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

		ConstantBuffer& GetConstantBuffer( int slot )
		{
			return m_ConstantBuffers[slot];
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pPixelShader;
		std::vector<ID3D11SamplerState*> m_pSamplerStates;
		std::vector<ConstantBuffer> m_ConstantBuffers;
	};
}