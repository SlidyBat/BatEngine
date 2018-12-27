#pragma once

#include "Event.h"

namespace Bat
{
	class KeyPressedEvent : public IEvent<KeyPressedEvent>
	{
	public:
		KeyPressedEvent( const size_t key, bool repeated )
			:
			m_iKey( key ),
			m_bRepeated( repeated )
		{}

		size_t GetKey() const { return m_iKey; }
		bool IsRepeated() const { return m_bRepeated; }
	private:
		size_t m_iKey;
		bool m_bRepeated;
	};

	class KeyReleasedEvent : public IEvent<KeyReleasedEvent>
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