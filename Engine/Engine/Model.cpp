#include "Model.h"

Model::~Model()
{
	if( m_pIndexBuffer )
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = nullptr;
	}

	if( m_pVertexBuffer )
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}
}

bool Model::Initialize( ID3D11Device* pDevice )
{
	return InitializeBuffers( pDevice );
}

void Model::Render( ID3D11DeviceContext* pDeviceContext )
{
	RenderBuffers( pDeviceContext );
}

int Model::GetIndexCount() const
{
	return m_nIndexCount;
}

bool Model::InitializeBuffers( ID3D11Device * pDevice )
{
	m_nVertexCount = 4;
	m_nIndexCount = 6;

	VertexType* vertices = new VertexType[m_nVertexCount];
	if( !vertices )
	{
		return false;
	}

	ULONG* indices = new ULONG[m_nIndexCount];
	if( !indices )
	{
		return false;
	}

	vertices[0].position = { -1.0f, -1.0f, 0.0f };
	vertices[0].colour = { 1.0f, 0.0f, 0.0f, 1.0f };
	
	vertices[1].position = { -1.0f, 1.0f, 0.0f };
	vertices[1].colour = { 1.0f, 0.0f, 0.0f, 1.0f };

	vertices[2].position = { 1.0f, 1.0f, 0.0f };
	vertices[2].colour = { 1.0f, 0.0f, 0.0f, 1.0f };

	vertices[3].position = { 1.0f, -1.0f, 0.0f };
	vertices[3].colour = { 1.0f, 0.0f, 0.0f, 1.0f };

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 3;
	indices[4] = 2;
	indices[5] = 0;

	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof( vertices[0] ) * m_nVertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	if( FAILED( pDevice->CreateBuffer( &vertexBufferDesc, &vertexData, &m_pVertexBuffer ) ) )
	{
		return false;
	}

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof( indices[0] ) * m_nVertexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	if( FAILED( pDevice->CreateBuffer( &indexBufferDesc, &indexData, &m_pIndexBuffer ) ) )
	{
		return false;
	}

	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}

void Model::RenderBuffers( ID3D11DeviceContext* pDeviceContext )
{
	UINT stride = sizeof( VertexType );
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );

	pDeviceContext->IASetIndexBuffer( m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );
	
	pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
}
