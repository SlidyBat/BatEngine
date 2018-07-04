#pragma once

#include <d3d11.h>
#include <wrl.h>

template<typename V>
class VertexBuffer
{
public:
	VertexBuffer( ID3D11Device* pDevice, const std::vector<V> vertices )
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof( V ) * vertices.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = vertices.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer( &vertexBufferDesc, &vertexData, &m_pVertexBuffer );
	}

	operator ID3D11Buffer*() const
	{
		return m_pVertexBuffer.Get();
	}
	
	ID3D11Buffer* const * GetAddressOf() const
	{
		return m_pVertexBuffer.GetAddressOf();
	}
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pVertexBuffer;
};