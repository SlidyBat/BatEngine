#include "StackAllocator.h"

#include <cassert>

namespace Bat
{
	StackAllocator::StackAllocator( size_t total_size )
		:
		m_pAllocatedData( new char[total_size] ),
		m_pAllocHead( m_pAllocatedData ),
		m_pEnd( m_pAllocatedData + total_size )
	{}

	StackAllocator::~StackAllocator()
	{
		assert( m_pAllocHead == m_pAllocatedData && "Memory leak detected" );
		delete[] m_pAllocatedData;
	}

	void* StackAllocator::Alloc( size_t size )
	{
		if( m_pAllocHead + size > m_pEnd )
		{
			assert( false && "Ran out of memory for stack allocator" );
			return nullptr;
		}

		char* pAlloc = m_pAllocHead;
		m_pAllocHead += size;
		return pAlloc;
	}

	void StackAllocator::ResetTo( const AllocMarker_t marker )
	{
		assert( marker > m_pAllocatedData );
		assert( marker < m_pEnd );
		m_pAllocHead = marker;
	}

	StackAllocator::AllocMarker_t StackAllocator::GetMarker() const
	{
		return m_pAllocHead;
	}
}