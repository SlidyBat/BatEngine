#include "Triangle.h"

Triangle::Triangle( ID3D11Device* pDevice, const std::array<Vertex, 3>& points )
	:
	triangleVerts( points )
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof( Vertex ) * 3;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = triangleVerts.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	pDevice->CreateBuffer( &vertexBufferDesc, &vertexData, &m_pVertexBuffer );
}

Triangle::~Triangle()
{
	if( m_pVertexBuffer )
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}
}

void Triangle::Render( ID3D11DeviceContext* pDeviceContext )
{
	RenderBuffers( pDeviceContext );
}

void Triangle::RenderBuffers( ID3D11DeviceContext* pDeviceContext )
{
	UINT stride = sizeof( Vertex );
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );
	
	pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
}
