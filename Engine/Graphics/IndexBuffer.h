#pragma once

#include <vector>
#include "Graphics.h"

namespace Bat
{
	template<typename T>
	class IndexBufferT
	{
	public:
		IndexBufferT() = default;
		IndexBufferT( const T* pData, const size_t size )
			:
			m_pIndexBuffer( gpu->CreateIndexBuffer( pData, sizeof( T ), size ) )
		{}
		IndexBufferT( const std::vector<T>& vertices )
			:
			IndexBufferT( vertices.data(), vertices.size() )
		{}

		void Reset( const T* pData, const size_t size )
		{
			m_pIndexBuffer.reset( gpu->CreateIndexBuffer( pData, sizeof( T ), size ) );
		}

		void Reset( const std::vector<T>& data )
		{
			Reset( data.data(), data.size() );
		}

		void Update( IGPUContext* pContext, const T* pData )
		{
			pContext->UpdateBuffer( m_pIndexBuffer.get(), pData );
		}

		T* Lock( IGPUContext* pContext )
		{
			void* pData = pContext->Lock( m_pIndexBuffer.get() );
			return reinterpret_cast<T*>(pData);
		}

		void Unlock( IGPUContext* pContext )
		{
			pContext->Unlock( m_pIndexBuffer.get() );
		}

		operator IIndexBuffer*() const
		{
			return m_pIndexBuffer.get();
		}

		IIndexBuffer* operator->()
		{
			return m_pIndexBuffer.get();
		}
		
		const IIndexBuffer* operator->() const
		{
			return m_pIndexBuffer.get();
		}
	private:
		std::unique_ptr<IIndexBuffer> m_pIndexBuffer = nullptr;
	};

	using IndexBuffer = IndexBufferT<uint32_t>;
}