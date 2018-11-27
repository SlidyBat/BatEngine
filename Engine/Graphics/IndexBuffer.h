#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "COMException.h"
#include "IGraphics.h"

namespace Bat
{
	class IndexBuffer
	{
	public:
		IndexBuffer() = default;
		IndexBuffer( const int* pData, const UINT size )
		{
			SetData( pData, size );
		}
		IndexBuffer( const std::vector<int>& indices )
			:
			IndexBuffer( indices.data(), (UINT)indices.size() )
		{}

		void SetData( const int* pData, const UINT size )
		{
			m_iSize = size;

			D3D11_BUFFER_DESC indexBufferDesc;
			indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			indexBufferDesc.ByteWidth = UINT( sizeof( unsigned long ) * size );
			indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexBufferDesc.CPUAccessFlags = 0;
			indexBufferDesc.MiscFlags = 0;
			indexBufferDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA indexData;
			indexData.pSysMem = pData;
			indexData.SysMemPitch = 0;
			indexData.SysMemSlicePitch = 0;

			COM_THROW_IF_FAILED( g_pGfx->GetDevice()->CreateBuffer( &indexBufferDesc, &indexData, &m_pIndexBuffer ) );
		}
		void SetData( const std::vector<int>& indices )
		{
			SetData( indices.data(), (UINT)indices.size() );
		}

		operator ID3D11Buffer*() const
		{
			return m_pIndexBuffer.Get();
		}

		void Bind() const
		{
			auto pDeviceContext = g_pGfx->GetDeviceContext();
			pDeviceContext->IASetIndexBuffer( m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0 );
		}

		UINT GetIndexCount() const
		{
			return m_iSize;
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pIndexBuffer;
		UINT m_iSize = 0;
	};
}