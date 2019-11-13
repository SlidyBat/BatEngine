#pragma once

#include "Graphics.h"

namespace Bat
{
	template<typename T>
	class ConstantBuffer
	{
	public:
		ConstantBuffer()
			:
			m_pConstantBuffer( gpu->CreateConstantBuffer( nullptr, sizeof( T ) ) )
		{
			static_assert( sizeof( T ) % 16 == 0, "Constant buffer struct is not 16 byte aligned!" );
		}
		ConstantBuffer( const T& data )
			:
			m_pConstantBuffer( gpu->CreateConstantBuffer( &data, sizeof( T ) ) )
		{
			static_assert( sizeof( T ) % 16 == 0, "Constant buffer struct is not 16 byte aligned!" );
		}

		operator IConstantBuffer*() const
		{
			return m_pConstantBuffer.get();
		}

		IConstantBuffer* operator->()
		{
			return m_pConstantBuffer.get();
		}

		void Update( IGPUContext* pContext, const T& data )
		{
			pContext->UpdateBuffer( m_pConstantBuffer.get(), &data );
		}

		T* Lock( IGPUContext* pContext )
		{
			void* pData = pContext->Lock( m_pConstantBuffer.get() );
			return reinterpret_cast<T*>(pData);
		}

		void Unlock( IGPUContext* pContext )
		{
			pContext->Unlock( m_pConstantBuffer.get() );
		}
	private:
		std::unique_ptr<IConstantBuffer> m_pConstantBuffer = nullptr;
	};
}