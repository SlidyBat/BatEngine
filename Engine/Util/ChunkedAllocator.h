#pragma once

#include <cassert>
#include <vector>

namespace Bat
{
	template <size_t ChunkSize = 8192>
	class ChunkedAllocator
	{
	public:
		ChunkedAllocator() = default;
		// The allocator will ensure at least this capacity, but may
		// also allocate more. Use Capacity() to check the real capacity.
		ChunkedAllocator( const size_t object_size, const size_t capacity = 1 )
			:
			m_iObjectSize( object_size )
		{
			if( capacity )
			{
				EnsureCapacity( capacity );
			}
		}
		~ChunkedAllocator()
		{
			for( char* pChunk : m_pChunks )
			{
				delete[] pChunk;
			}
		}

		ChunkedAllocator( const ChunkedAllocator& ) = delete;
		ChunkedAllocator& operator=( const ChunkedAllocator& ) = delete;

		ChunkedAllocator( ChunkedAllocator&& donor )
		{
			*this = std::move( donor );
		}
		ChunkedAllocator& operator=( ChunkedAllocator&& rhs )
		{
			m_iCapacity = rhs.m_iCapacity;
			m_pChunks = std::move( rhs.m_pChunks );
			m_iObjectSize = rhs.m_iObjectSize;
			return *this;
		}

		// Returns pointer to the object at given index
		// Not guaranteed to be a valid object
		void* Get( size_t index )
		{
			return m_pChunks[index / ChunkSize] + ( index % ChunkSize ) * m_iObjectSize;
		}

		void EnsureCapacity( const size_t capacity )
		{
			ASSERT( capacity >= 0, "Cannot ensure 0 capacity" );
			if( m_iCapacity >= capacity )
			{
				return;
			}

			const size_t nChunksNeeded = ( ( capacity - 1 ) / ChunkSize ) + 1;
			const size_t nOldChunksNeeded = m_iCapacity ? ( ( ( m_iCapacity - 1 ) / ChunkSize ) + 1 ) : 0;

			m_pChunks.resize( nChunksNeeded );
			for( size_t i = nOldChunksNeeded; i < nChunksNeeded; i++ )
			{
				m_pChunks[i] = new char[ChunkSize * m_iObjectSize];
			}

			m_iCapacity = nChunksNeeded * ChunkSize;
		}

		size_t Capacity() const { return m_iCapacity; }
	public:
		static constexpr size_t GROW_MULTIPLIER = 2;

		std::vector<char*> m_pChunks;
		size_t m_iCapacity = 0;

		size_t m_iObjectSize = 0;
	};
}