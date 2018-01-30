#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <array>
#include <wrl.h>

template <typename V>
class Triangle
{
public:
	Triangle( ID3D11Device* pDevice, const std::array<V, 3>& points )
		:
		triangleVerts( points )
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof( V ) * 3;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = triangleVerts.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer( &vertexBufferDesc, &vertexData, &m_pVertexBuffer );

		unsigned long indexes[3] = { 0, 1, 2 };

		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof( unsigned long ) * 3;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexData;
		indexData.pSysMem = &indexes;
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer( &indexBufferDesc, &indexData, &m_pIndexBuffer );
	}
	Triangle( ID3D11Device* pDevice, V v1, V v2, V v3 )
		:
		Triangle( pDevice, std::array<V, 3>{ v1, v2, v3 } )
	{}
	Triangle( const Triangle& src ) = delete;
	Triangle& operator=( const Triangle& src ) = delete;
	Triangle( Triangle&& donor ) = delete;
	Triangle& operator=( Triangle&& donor ) = delete;

	void Render( ID3D11DeviceContext* pDeviceContext )
	{
		RenderBuffers( pDeviceContext );
	}
	static int GetIndexCount()
	{
		return 3;
	}
private:
	void RenderBuffers( ID3D11DeviceContext* pDeviceContext )
	{
		UINT stride = sizeof( V );
		UINT offset = 0;
		pDeviceContext->IASetVertexBuffers( 0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset );
		pDeviceContext->IASetIndexBuffer( m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0 );

		pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	}
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pIndexBuffer;

	std::array<V, 3>						triangleVerts;
};