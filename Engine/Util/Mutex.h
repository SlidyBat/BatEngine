#pragma once

#include "Platform/BatWinAPI.h"

namespace Bat
{
	class Mutex
	{
	public:
		Mutex();
		~Mutex();

		Mutex( const Mutex& ) = delete;
		Mutex& operator=( const Mutex& ) = delete;
		Mutex( Mutex&& ) = delete;
		Mutex& operator=( Mutex&& ) = delete;

		void Lock();
		void Unlock();
		bool TryLock();
	private:
		CRITICAL_SECTION m_CritSection;
	};

	class ScopedLock
	{
	public:
		ScopedLock( Mutex& mutex );
		~ScopedLock();

		ScopedLock( const ScopedLock& ) = delete;
		ScopedLock& operator=( const ScopedLock& ) = delete;
		ScopedLock( ScopedLock&& ) = delete;
		ScopedLock& operator=( ScopedLock&& ) = delete;
	private:
		Mutex& m_Mutex;
	};
}