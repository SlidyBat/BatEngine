#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "COMException.h"
#include "IGraphics.h"

namespace Bat
{
	template<typename V>
	class VertexBuffer
	{
	public:
		VertexBuffer() = default;
		VertexBuffer( const V* pData, const UINT size )
		{
			SetData( pData, size );
		}
		VertexBuffer( const std::vector<V>& vertices )
			:
			VertexBuffer( vertices.data(), (UINT)vertices.size() )
		{}

		void SetData( const V* pData, const UINT size )
		{
			m_iSize = size;
			auto pDevice = g_pGfx->GetDevice();

			D3D11_BUFFER_DESC vertexBufferDesc;
			vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			vertexBufferDesc.ByteWidth = UINT( sizeof( V ) * size );
			vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexBufferDesc.CPUAccessFlags = 0;
			vertexBufferDesc.MiscFlags = 0;
			vertexBufferDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA vertexData;
			vertexData.pSysMem = pData;
			vertexData.SysMemPitch = 0;
			vertexData.SysMemSlicePitch = 0;

			COM_THROW_IF_FAILED( pDevice->CreateBuffer( &vertexBufferDesc, &vertexData, &m_pVertexBuffer ) );
		}

		void SetData( const std::vector<V>& vertices )
		{
			SetData( vertices.data(), vertices.size() );
		}

		operator ID3D11Buffer*() const
		{
			return m_pVertexBuffer.Get();
		}

		ID3D11Buffer* const * GetAddressOf() const
		{
			return m_pVertexBuffer.GetAddressOf();
		}

		size_t GetVertexCount() const
		{
			return (size_t)m_iSize;
		}

		void Bind( UINT slot ) const
		{
			UINT stride = sizeof( V );
			UINT offset = 0;
			g_pGfx->GetDeviceContext()->IASetVertexBuffers( slot, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset );
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer = nullptr;
		UINT m_iSize = 0;
	};
}