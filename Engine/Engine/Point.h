#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <wrl.h>

template <typename V>
class Point
{
public:
	Point( ID3D11Device* pDevice, const std::vector<V>& points )
		:
		pointVerts( points )
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof( V );
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = pointVerts.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer( &vertexBufferDesc, &vertexData, &m_pVertexBuffer );
	}
	Point( ID3D11Device* pDevice, V point )
		:
		Point( pDevice, std::vector<V>{ point } )
	{}
	Point( const Point& src ) = delete;
	Point& operator=( const Point& src ) = delete;
	Point( Point&& donor ) = delete;
	Point& operator=( Point&& donor ) = delete;

	void Render( ID3D11DeviceContext* pDeviceContext )
	{
		RenderBuffers( pDeviceContext );
	}
private:
	void RenderBuffers( ID3D11DeviceContext* pDeviceContext )
	{
		UINT stride = sizeof( V );
		UINT offset = 0;
		pDeviceContext->IASetVertexBuffers( 0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset );

		pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
	}
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pVertexBuffer;

	std::vector<V>							pointVerts;
};