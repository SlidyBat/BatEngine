#pragma once

namespace Bat
{
	class StackAllocator
	{
	public:
		using AllocMarker_t = char*;

		StackAllocator( size_t total_size );
		~StackAllocator();

		void* Alloc( size_t size );
		template <typename T, typename... Args>
		T* Alloc( Args&&... args )
		{
			void* pMem = Alloc( sizeof( T ) );
			return new(pMem) T( std::forward<Args>( args )... );
		}
		template <typename T>
		void Free( T* pObj )
		{
			pObj->~T();
			m_pAllocHead -= sizeof( T );
		}
		void ResetTo( const AllocMarker_t marker );
		AllocMarker_t GetMarker() const;
	private:
		char* m_pAllocatedData = nullptr;
		char* m_pEnd = nullptr;
		char* m_pAllocHead = nullptr;
	};
}