#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "COMException.h"

namespace Bat
{
	class IndexBuffer
	{
	public:
		IndexBuffer( ID3D11Device* pDevice, const int* data, const UINT size )
			:
			size( size )
		{
			D3D11_BUFFER_DESC indexBufferDesc;
			indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			indexBufferDesc.ByteWidth = UINT( sizeof( unsigned long ) * size );
			indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexBufferDesc.CPUAccessFlags = 0;
			indexBufferDesc.MiscFlags = 0;
			indexBufferDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA indexData;
			indexData.pSysMem = data;
			indexData.SysMemPitch = 0;
			indexData.SysMemSlicePitch = 0;

			COM_ERROR_IF_FAILED( pDevice->CreateBuffer( &indexBufferDesc, &indexData, &m_pIndexBuffer ) );
		}
		IndexBuffer( ID3D11Device* pDevice, const std::vector<int>& indices )
			:
			IndexBuffer( pDevice, indices.data(), (UINT)indices.size() )
		{}

		operator ID3D11Buffer*() const
		{
			return m_pIndexBuffer.Get();
		}

		void Bind( ID3D11DeviceContext* pDeviceContext ) const
		{
			pDeviceContext->IASetIndexBuffer( m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0 );
		}

		UINT GetIndexCount() const
		{
			return size;
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pIndexBuffer;
		UINT size;
	};
}