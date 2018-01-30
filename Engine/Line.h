#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <array>
#include <wrl.h>

template <typename V>
class Line
{
public:
	Line( ID3D11Device* pDevice, const std::array<V, 2>& points )
		:
		lineVerts( points )
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof( V ) * 2;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = lineVerts.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer( &vertexBufferDesc, &vertexData, &m_pVertexBuffer );

		unsigned long indexes[2] = { 0, 1 };

		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof( unsigned long ) * 2;
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
	Line( ID3D11Device* pDevice, V v1, V v2 )
		:
		Line( pDevice, std::array<V, 2>{ v1, v2 } )
	{}
	Line( const Line& src ) = delete;
	Line& operator=( const Line& src ) = delete;
	Line( Line&& donor ) = delete;
	Line& operator=( Line&& donor ) = delete;

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

		pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
	}
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pIndexBuffer;

	std::array<V, 2>						lineVerts;
};