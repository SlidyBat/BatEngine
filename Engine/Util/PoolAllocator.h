#pragma once

#include <cassert>

namespace Bat
{
	template <typename T>
	class PoolAllocator
	{
	public:
		PoolAllocator( const size_t capacity )
		{
			// we double up the memory as a linked list that stores 1 pointer per node
			// so we need at least sizeof a pointer for this to work
			static_assert(sizeof( T ) >= sizeof( char* ));
			assert( capacity > 0 );

			if( capacity )
			{
				m_iCapacity = capacity;
				m_pMemPool = new char[sizeof( T ) * capacity];
				char* pCurrent = m_pMemPool;
				m_pFreeList = pCurrent;
				for( size_t i = 1; i < capacity; i++ )
				{
					char* pNew = pCurrent + sizeof( T );
					*(char**)pCurrent = pNew;
					pCurrent = pNew;
				}
				*(char**)pCurrent = nullptr;
			}
		}
		~PoolAllocator()
		{
			delete[] m_pMemPool;

#ifdef _DEBUG
			if( m_iAllocs )
			{
				assert( false && "Memory leak detected" );
			}
#endif
		}

		PoolAllocator( const PoolAllocator& ) = delete;
		PoolAllocator& operator=( const PoolAllocator& ) = delete;

		PoolAllocator( PoolAllocator&& donor )
		{
			*this = std::move( donor );
		}
		PoolAllocator& operator=( const PoolAllocator&& rhs )
		{
			m_iCapacity = rhs.m_iCapacity;
			m_pMemPool = rhs.m_pMemPool;
			m_pFreeList = rhs.m_pFreeList;
#
			rhs.m_pMemPool = nullptr;
#ifdef _DEBUG
			m_iAllocs = rhs.m_iAllocs;
			rhs.m_iAllocs = 0;
#endif
		}

		template <typename... Args>
		T* Alloc( Args&&... args )
		{
			if( !m_pFreeList )
			{
				assert( false && "Pool allocator ran out of memory" );
				return nullptr;
			}

#ifdef _DEBUG
			m_iAllocs++;
#endif

			char* pAlloc = m_pFreeList;
			m_pFreeList = *(char**)m_pFreeList;
			return new(pAlloc) T( std::forward<Args>( args )... );
		}
		void Free( T* pObject )
		{
			pObject->~T();
			*(char**)pObject = m_pFreeList;
			m_pFreeList = (char*)pObject;

#ifdef _DEBUG
			m_iAllocs--;
#endif
		}
	private:
		char* m_pMemPool = nullptr;
		char* m_pFreeList = nullptr;
		size_t m_iCapacity = 0;
#ifdef _DEBUG
		size_t m_iAllocs = 0;
#endif
	};
}