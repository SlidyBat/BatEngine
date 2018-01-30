#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <array>
#include <wrl.h>

template <typename V>
class Quad
{
public:
	Quad( ID3D11Device* pDevice, const std::array<V, 4>& points )
		:
		quadVerts( points )
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof( V ) * 4;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = quadVerts.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer( &vertexBufferDesc, &vertexData, &m_pVertexBuffer );

		unsigned long indexes[6] = { 0, 1, 2, 2, 3, 0 };

		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof( unsigned long ) * 6;
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
	Quad( ID3D11Device* pDevice, V v1, V v2, V v3, V v4 )
		:
		Quad( pDevice, std::array<V, 4>{ v1, v2, v3, v4 } )
	{}
	Quad( const Quad& src ) = delete;
	Quad& operator=( const Quad& src ) = delete;
	Quad( Quad&& donor ) = delete;
	Quad& operator=( Quad&& donor ) = delete;

	void Render( ID3D11DeviceContext* pDeviceContext )
	{
		RenderBuffers( pDeviceContext );
	}
	static int GetIndexCount()
	{
		return 6;
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

	std::array<V, 4>						quadVerts;
};