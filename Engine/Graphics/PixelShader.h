#pragma once

#pragma once

#include <d3d11.h>
#include <vector>
#include <string>
#include <wrl.h>

#include "ConstantBuffer.h"

namespace Bat
{
	class PixelShader
	{
	public:
		PixelShader( const std::wstring& filename );
		~PixelShader();

		void Bind();
		void AddSampler( const D3D11_SAMPLER_DESC* pSamplerDesc );
		void SetResource( int slot, ID3D11ShaderResourceView* const pResource );
		void SetResources( int startslot, ID3D11ShaderResourceView** const pResource, size_t size );

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