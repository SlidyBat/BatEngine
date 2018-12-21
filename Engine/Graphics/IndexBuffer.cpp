#include "PCH.h"
#include "IndexBuffer.h"

#include "RenderContext.h"

namespace Bat
{
	IndexBuffer::IndexBuffer( const int * pData, const UINT size )
	{
		SetData( pData, size );
	}

	IndexBuffer::IndexBuffer( const std::vector<int>& indices )
		:
		IndexBuffer( indices.data(), (UINT)indices.size() )
	{}

	void IndexBuffer::SetData( const int * pData, const UINT size )
	{
		m_iSize = size;

		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = UINT( sizeof( unsigned long ) * size );
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexData;
		indexData.pSysMem = pData;
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		COM_THROW_IF_FAILED( RenderContext::GetDevice()->CreateBuffer( &indexBufferDesc, &indexData, &m_pIndexBuffer ) );
	}

	void IndexBuffer::SetData( const std::vector<int>& indices )
	{
		SetData( indices.data(), (UINT)indices.size() );
	}

	IndexBuffer::operator ID3D11Buffer*( ) const
	{
		return m_pIndexBuffer.Get();
	}
	void IndexBuffer::Bind() const
	{
		auto pDeviceContext = RenderContext::GetDeviceContext();
		pDeviceContext->IASetIndexBuffer( m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0 );
	}
	UINT IndexBuffer::GetIndexCount() const
	{
		return m_iSize;
	}
}