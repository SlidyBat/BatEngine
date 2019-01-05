#pragma once

#include "PCH.h"
#include <wrl.h>

#include "ConstantBuffer.h"
#include "VertexTypes.h"

namespace Bat
{
	class VertexShader
	{
	public:
		VertexShader( const std::string& filename );

		void Bind();
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

		void LoadFromFile( const std::wstring& filename, bool crash_on_error );
		void OnFileChanged( const std::string& filename );
		bool IsDirty() const { return m_bDirty; }
		void SetDirty( bool dirty ) { m_bDirty = dirty; }
	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_pVertexShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_pInputLayout;
		bool m_bUsesAttribute[(int)VertexAttribute::TotalAttributes];
		std::vector<ConstantBuffer> m_ConstantBuffers;
		std::string m_szFilename;
		std::atomic_bool m_bDirty;
	};
}