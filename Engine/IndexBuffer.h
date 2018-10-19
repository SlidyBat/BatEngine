#pragma once

#include <d3d11.h>
#include <wrl.h>

class IndexBuffer
{
public:
	IndexBuffer( ID3D11Device* pDevice, const std::vector<int>& indices )
	{
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = UINT( sizeof( unsigned long ) * indices.size() );
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexData;
		indexData.pSysMem = indices.data();
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer( &indexBufferDesc, &indexData, &m_pIndexBuffer );
	}

	operator ID3D11Buffer*() const
	{
		return m_pIndexBuffer.Get();
	}
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pIndexBuffer;
};