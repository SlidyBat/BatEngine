#include "PCH.h"
#include "Mutex.h"

namespace Bat
{
	Mutex::Mutex()
	{
		InitializeCriticalSection( &m_CritSection );
	}

	Mutex::~Mutex()
	{
		DeleteCriticalSection( &m_CritSection );
	}

	void Mutex::Lock()
	{
		EnterCriticalSection( &m_CritSection );
	}

	void Mutex::Unlock()
	{
		LeaveCriticalSection( &m_CritSection );
	}

	bool Mutex::TryLock()
	{
		return TryEnterCriticalSection( &m_CritSection );
	}

	ScopedLock::ScopedLock( Mutex& mutex )
		:
		m_Mutex( mutex )
	{
		m_Mutex.Lock();
	}

	ScopedLock::~ScopedLock()
	{
		m_Mutex.Unlock();
	}
}