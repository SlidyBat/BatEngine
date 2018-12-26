#pragma once

#include <cassert>

namespace Bat
{
	template <typename T, size_t ChunkSize = 8192, bool GrowOnOverflow = true>
	class PoolAllocator
	{
	public:
		// The allocator will ensure at least this capacity, but may
		// also allocate more. Use Capacity() to check the real capacity.
		PoolAllocator( const size_t capacity = 1 )
		{
			// we double up the memory as a linked list that stores 1 pointer per node
			// so we need at least sizeof a pointer for this to work
			static_assert(sizeof( T ) >= sizeof( char* ));
			// not a good idea to have a size bigger than chunk size
			static_assert( sizeof( T ) < ChunkSize );
			assert( capacity > 0 );

			if( capacity )
			{
				const size_t nBytes = sizeof( T ) * capacity;
				m_iCapacity = ChunkSize / nBytes;
				const size_t nChunksNeeded = (nBytes / ChunkSize) + 1;
				m_pChunks.resize( nChunksNeeded );
				for( size_t i = 0; i < nChunksNeeded; i++ )
				{
					m_pChunks[i] = new char[ChunkSize];
				}

				m_pAllocHead = m_pChunks[0];
			}
		}
		~PoolAllocator()
		{
			for( char* pChunk : m_pChunks )
			{
				delete[] pChunk;
			}

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
			char* pAlloc = AllocInternal();

			if( !pAlloc )
			{
				if( GrowOnOverflow )
				{
					EnsureCapacity( m_iCapacity * GROW_MULTIPLIER ); // grow to accomodate
					pAlloc = AllocInternal(); // try again
					if( !pAlloc )
					{
						assert( false && "Failed to grow" );
						return nullptr;
					}
				}
				else
				{
					assert( false && "Ran out of memory" );
					return nullptr;
				}
			}

#ifdef _DEBUG
			m_iAllocs++;
#endif
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

		void EnsureCapacity( const size_t capacity )
		{
			if( m_iCapacity >= capacity )
			{
				return;
			}

			const size_t nBytes = capacity * sizeof( T );
			const size_t nChunksNeeded = (nBytes / ChunkSize) + 1;
			const size_t nOldBytes = m_iCapacity * sizeof( T );
			const size_t nOldChunksNeeded = (nOldBytes / ChunkSize) + 1;

			m_pChunks.resize( nChunksNeeded );
			for( size_t i = nOldChunksNeeded; i < nChunksNeeded; i++ )
			{
				m_pChunks[i] = new char[ChunkSize];
			}

			m_iCapacity = capacity;
		}

		size_t Capacity() const { return m_iCapacity; }
	private:
		char* AllocInternal()
		{
			char* pAlloc = nullptr;

			if( m_pFreeList )
			{
				pAlloc = m_pFreeList;
				m_pFreeList = *(char**)m_pFreeList;
			}
			else
			{
				pAlloc = m_pAllocHead;

				m_pAllocHead += sizeof( T );
				const size_t nBytesUsedInChunk = m_pAllocHead - m_pChunks[m_iCurrentChunk];
				if( nBytesUsedInChunk >= ChunkSize )
				{
					m_pAllocHead = m_pChunks[++m_iCurrentChunk]; // move on to next chunk

					if( nBytesUsedInChunk > ChunkSize )
					{
						// there wasn't enough memory in that last chunk to do the allocation
						pAlloc = m_pAllocHead;
						// TODO: unlikely to happen, but should check for chunk change again
						m_pAllocHead += sizeof( T );
					}
				}
			}

			return pAlloc;
		}
	private:
		static constexpr size_t GROW_MULTIPLIER = 2;

		std::vector<char*> m_pChunks;
		size_t m_iCurrentChunk;
		char* m_pAllocHead = nullptr;
		char* m_pFreeList = nullptr;
		size_t m_iCapacity = 0;
#ifdef _DEBUG
		size_t m_iAllocs = 0;
#endif
	};
}