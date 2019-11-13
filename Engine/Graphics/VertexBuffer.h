#pragma once

#include "Graphics.h"

namespace Bat
{
	template<typename V>
	class VertexBuffer
	{
	public:
		VertexBuffer() = default;
		VertexBuffer( const V* pData, const size_t size )
			:
			m_pVertexBuffer( gpu->CreateVertexBuffer( pData, sizeof( V ), size ) )
		{}
		VertexBuffer( const std::vector<V>& vertices )
			:
			VertexBuffer( vertices.data(), vertices.size() )
		{}

		void Reset( const V* pData, const size_t size )
		{
			m_pVertexBuffer.reset( gpu->CreateVertexBuffer( pData, sizeof( V ), size ) );
		}

		void Reset( const std::vector<V>& data )
		{
			Reset( data.data(), data.size() );
		}

		void Update( IGPUContext* pContext, const V* pData )
		{
			pContext->UpdateBuffer( m_pVertexBuffer.get(), pData );
		}

		V* Lock( IGPUContext* pContext )
		{
			void* pData = pContext->Lock( m_pVertexBuffer.get() );
			return reinterpret_cast<V*>(pData);
		}

		void Unlock( IGPUContext* pContext )
		{
			pContext->Unlock( m_pVertexBuffer.get() );
		}

		operator IVertexBuffer*() const
		{
			return m_pVertexBuffer.get();
		}

		IVertexBuffer* operator->()
		{
			return m_pVertexBuffer.get();
		}

		const IVertexBuffer* operator->() const
		{
			return m_pVertexBuffer.get();
		}
	private:
		std::unique_ptr<IVertexBuffer> m_pVertexBuffer = nullptr;
	};
}