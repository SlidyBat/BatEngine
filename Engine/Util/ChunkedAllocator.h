#pragma once

#include <cassert>
#include <vector>

namespace Bat
{
	class ChunkedAllocator
	{
	public:
		// The allocator will ensure at least this capacity, but may
		// also allocate more. Use Capacity() to check the real capacity.
		ChunkedAllocator( const size_t object_size, const size_t chunk_size, const size_t capacity = 1 )
			:
			m_iObjectSize( object_size ),
			m_iChunkSize( chunk_size )
		{
			ASSERT( capacity > 0, "Cannot have 0 capacity" );

			const size_t nBytes = object_size * capacity;
			m_iCapacity = chunk_size / nBytes;
			const size_t nChunksNeeded = (nBytes / chunk_size) + 1;
			m_pChunks.resize( nChunksNeeded );
			for( size_t i = 0; i < nChunksNeeded; i++ )
			{
				m_pChunks[i] = new char[chunk_size];
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
			m_iCurrentChunk = rhs.m_iCurrentChunk;
			m_iObjectSize = rhs.m_iObjectSize;
		}

		// Returns pointer to the object at given index
		// Not guaranteed to be a valid object
		void* Get( size_t index )
		{
			size_t elements_per_chunk = m_iChunkSize / m_iObjectSize;
			return m_pChunks[index / elements_per_chunk] + ( index % elements_per_chunk ) * m_iObjectSize;
		}

		void EnsureCapacity( const size_t capacity )
		{
			ASSERT( capacity >= 0, "Cannot ensure 0 capacity" );
			if( m_iCapacity >= capacity )
			{
				return;
			}

			const size_t nBytes = capacity * m_iObjectSize;
			const size_t nChunksNeeded = ((nBytes - 1) / m_iChunkSize) + 1;
			const size_t nOldBytes = m_iCapacity * m_iObjectSize;
			const size_t nOldChunksNeeded = ((nOldBytes - 1) / m_iChunkSize) + 1;

			m_pChunks.resize( nChunksNeeded );
			for( size_t i = nOldChunksNeeded; i < nChunksNeeded; i++ )
			{
				m_pChunks[i] = new char[m_iChunkSize];
			}

			size_t elements_per_chunk = m_iChunkSize / m_iObjectSize;
			m_iCapacity = nChunksNeeded * elements_per_chunk;
		}

		size_t Capacity() const { return m_iCapacity; }
	public:
		static constexpr size_t GROW_MULTIPLIER = 2;

		std::vector<char*> m_pChunks;
		size_t m_iCurrentChunk = 0;
		size_t m_iCapacity = 0;

		size_t m_iObjectSize = 0;
		size_t m_iChunkSize = 0;
	};

	template <typename T, size_t ChunkSize = 8192>
	class ObjectChunkedAllocator : public ChunkedAllocator
	{
	public:
		ObjectChunkedAllocator( const size_t capacity = 1 )
			:
			ChunkedAllocator( sizeof( T ), ChunkSize * sizeof( T ), capacity )
		{
			// we double up the memory as a linked list that stores 1 pointer per node
			// so we need at least sizeof a pointer for this to work
			static_assert( sizeof( T ) >= sizeof( char* ) );
			// not a good idea to have a size bigger than chunk size
			static_assert( sizeof( T ) < ChunkSize );
		}

		T* Get( size_t index )
		{
			return reinterpret_cast<T*>( ChunkedAllocator::Get( index ) );
		}

		const T* Get( size_t index ) const
		{
			return reinterpret_cast<const T*>(ChunkedAllocator::Get( index ));
		}
	};
}