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
		PixelShader( const std::string& filename );

		void Bind();
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
		void LoadFromFile( const std::wstring& filename, bool crash_on_error );
		void OnFileChanged( const std::string& filename );
		bool IsDirty() const { return m_bDirty; }
		void SetDirty( bool dirty ) { m_bDirty = dirty; }
	private:
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pPixelShader;
		std::vector<ConstantBuffer> m_ConstantBuffers;
		std::string m_szFilename;
		std::atomic_bool m_bDirty;
	};
}