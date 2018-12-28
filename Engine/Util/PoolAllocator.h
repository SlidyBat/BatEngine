#pragma once

#include <cassert>

namespace Bat
{
	class PoolAllocator
	{
	public:
		// The allocator will ensure at least this capacity, but may
		// also allocate more. Use Capacity() to check the real capacity.
		PoolAllocator( const size_t object_size, const size_t chunk_size, const size_t capacity = 1, bool grow_on_overflow = true )
			:
			m_iObjectSize( object_size ),
			m_iChunkSize( chunk_size ),
			m_bGrowOnOverflow( grow_on_overflow )
		{
			assert( capacity > 0 );

			if( capacity )
			{
				const size_t nBytes = object_size * capacity;
				m_iCapacity = chunk_size / nBytes;
				const size_t nChunksNeeded = (nBytes / chunk_size) + 1;
				m_pChunks.resize( nChunksNeeded );
				for( size_t i = 0; i < nChunksNeeded; i++ )
				{
					m_pChunks[i] = new char[chunk_size];
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
		PoolAllocator& operator=( PoolAllocator&& rhs )
		{
			m_iCapacity = rhs.m_iCapacity;
			m_pChunks = std::move( rhs.m_pChunks );
			m_iCurrentChunk = rhs.m_iCurrentChunk;
			m_pFreeList = rhs.m_pFreeList;
			m_pAllocHead = rhs.m_pAllocHead;

#ifdef _DEBUG
			m_iAllocs = rhs.m_iAllocs;
			rhs.m_iAllocs = 0;
#endif
		}

		void EnsureCapacity( const size_t capacity )
		{
			if( m_iCapacity >= capacity )
			{
				return;
			}

			const size_t nBytes = capacity * m_iObjectSize;
			const size_t nChunksNeeded = (nBytes / m_iChunkSize) + 1;
			const size_t nOldBytes = m_iCapacity * m_iObjectSize;
			const size_t nOldChunksNeeded = (nOldBytes / m_iChunkSize) + 1;

			m_pChunks.resize( nChunksNeeded );
			for( size_t i = nOldChunksNeeded; i < nChunksNeeded; i++ )
			{
				m_pChunks[i] = new char[m_iChunkSize];
			}

			m_iCapacity = capacity;
		}

		size_t Capacity() const { return m_iCapacity; }
	protected:
		void* AllocObject()
		{
			char* pAlloc = AllocInternal();

			if( !pAlloc )
			{
				if( m_bGrowOnOverflow )
				{
					EnsureCapacity( m_iCapacity * GROW_MULTIPLIER ); // grow to accomodate
					pAlloc = AllocInternal(); // try again
					assert( pAlloc && "Failed to grow" );
				}
				else
				{
					assert( false && "Ran out of memory" );
				}
			}

#ifdef _DEBUG
			m_iAllocs++;
#endif
			return pAlloc;
		}

		void FreeObject( void* pObject )
		{
			*(char**)pObject = m_pFreeList;
			m_pFreeList = (char*)pObject;

#ifdef _DEBUG
			m_iAllocs--;
#endif
		}
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

				m_pAllocHead += m_iObjectSize;
				const size_t nBytesUsedInChunk = m_pAllocHead - m_pChunks[m_iCurrentChunk];
				if( nBytesUsedInChunk >= m_iChunkSize )
				{
					m_pAllocHead = m_pChunks[++m_iCurrentChunk]; // move on to next chunk

					if( nBytesUsedInChunk > m_iChunkSize )
					{
						// there wasn't enough memory in that last chunk to do the allocation
						pAlloc = m_pAllocHead;
						// TODO: unlikely to happen, but should check for chunk change again
						m_pAllocHead += m_iObjectSize;
					}
				}
			}

			return pAlloc;
		}
	protected:
		static constexpr size_t GROW_MULTIPLIER = 2;

		std::vector<char*> m_pChunks;
		size_t m_iCurrentChunk = 0;
		char* m_pAllocHead = nullptr;
		char* m_pFreeList = nullptr;
		size_t m_iCapacity = 0;
#ifdef _DEBUG
		size_t m_iAllocs = 0;
#endif

		size_t m_iObjectSize = 0;
		size_t m_iChunkSize = 0;
		bool m_bGrowOnOverflow = false;
	};

	template <typename T, size_t ChunkSize = 8192, bool GrowOnOverflow = true>
	class ObjectAllocator : public PoolAllocator
	{
	public:
		ObjectAllocator( const size_t capacity = 1 )
			:
			PoolAllocator( sizeof( T ), ChunkSize * sizeof( T ), capacity, GrowOnOverflow )
		{
			// we double up the memory as a linked list that stores 1 pointer per node
			// so we need at least sizeof a pointer for this to work
			static_assert( sizeof( T ) >= sizeof( char* ) );
			// not a good idea to have a size bigger than chunk size
			static_assert( sizeof( T ) < ChunkSize );
		}

		template <typename... Args>
		T* Alloc( Args&&... args )
		{
			void* pAlloc = AllocObject();
			return new( pAlloc ) T( std::forward<Args>( args )... );
		}

		void Free( T* pObject )
		{
			pObject->~T();
			FreeObject( (void*)pObject );
		}

		// Returns pointer to the object at given index
		// Not guaranteed to be a valid object
		T* Get( size_t index )
		{
			size_t elements_per_chunk = m_iChunkSize / sizeof( T );
			return m_pChunks[index / elements_per_chunk] + ( index % elements_per_chunk ) * sizeof( T );
		}
	};
}