#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "VertexBuffer.h"
#include "IndexBuffer.h"

template <typename V>
class Model
{
public:
	Model( ID3D11Device* pDevice, const std::vector<V>& verts, const std::vector<int>& indices, D3D_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST )
		:
		m_Vertices( verts ),
		m_Indices( indices ),
		m_PrimitiveTopology( topology ),
		m_IndexBuffer( pDevice, indices ),
		m_VertexBuffer( pDevice, verts )
	{}

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
		pDeviceContext->IASetVertexBuffers( 0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset );
		pDeviceContext->IASetIndexBuffer( m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

		pDeviceContext->IASetPrimitiveTopology( m_PrimitiveTopology );
	}
private:
	VertexBuffer<V> m_VertexBuffer;
	IndexBuffer m_IndexBuffer;

	D3D_PRIMITIVE_TOPOLOGY					m_PrimitiveTopology;
	std::vector<V>							m_Vertices;
	std::vector<int>						m_Indices;
};