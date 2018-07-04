#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <wrl.h>

template <typename V>
class Model
{
public:
	Model( ID3D11Device* pDevice, const std::vector<V>& verts, const std::vector<int>& indices, D3D_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST )
		:
		m_Vertices( verts ),
		m_Indices( indices ),
		m_PrimitiveTopology( topology )
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof( V ) * verts.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = verts.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer( &vertexBufferDesc, &vertexData, &m_pVertexBuffer );

		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof( unsigned long ) * indices.size();
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

	Model( const Model& src ) = delete;
	Model& operator=( const Model& src ) = delete;
	Model( Model&& donor ) = delete;
	Model& operator=( Model&& donor ) = delete;

	void Render( ID3D11DeviceContext* pDeviceContext )
	{
		RenderBuffers( pDeviceContext );
	}

	int GetIndexCount() const
	{
		return m_Indices.size();
	}
	int GetVertexCount() const
	{
		return m_Vertices.size();
	}
	D3D_PRIMITIVE_TOPOLOGY GetTopologyType() const
	{
		return m_PrimitiveTopology;
	}
private:
	void RenderBuffers( ID3D11DeviceContext* pDeviceContext )
	{
		UINT stride = sizeof( V );
		UINT offset = 0;
		pDeviceContext->IASetVertexBuffers( 0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset );
		pDeviceContext->IASetIndexBuffer( m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0 );

		pDeviceContext->IASetPrimitiveTopology( m_PrimitiveTopology );
	}
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pIndexBuffer;

	D3D_PRIMITIVE_TOPOLOGY					m_PrimitiveTopology;
	std::vector<V>							m_Vertices;
	std::vector<int>						m_Indices;
};