#pragma once

#include "Event.h"

namespace Bat
{
	class KeyPressedEvent : public IEvent
	{
	public:
		KeyPressedEvent( const size_t key )
			:
			m_iKey( key )
		{}

		size_t GetKey() const { return m_iKey; }
	private:
		size_t m_iKey;
	};

	class KeyReleasedEvent : public IEvent
	{
	public:
		KeyReleasedEvent( const size_t key )
			:
			m_iKey( key )
		{}

		size_t GetKey() const { return m_iKey; }
	private:
		size_t m_iKey;
	};
}