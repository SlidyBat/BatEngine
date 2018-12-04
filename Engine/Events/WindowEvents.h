#pragma once

#include "BatWinAPI.h"
#include "Event.h"

namespace Bat
{
	class WindowResizeEvent : public IEvent
	{
	public:
		WindowResizeEvent( int width, int height )
			:
			m_iWidth( width ),
			m_iHeight( height )
		{}

		int GetWidth() const { return m_iWidth; }
		int GetHeight() const { return m_iHeight; }
	private:
		int m_iWidth, m_iHeight;
	};

	class WindowMovedEvent : public IEvent
	{
	public:
		WindowMovedEvent( int x, int y )
			:
			m_iX( x ),
			m_iY( y )
		{}

		int GetPosX() const { return m_iX; }
		int GetPosY() const { return m_iY; }
	private:
		int m_iX, m_iY;
	};

	class WindowClosedEvent : public IEvent
	{
	public:
		WindowClosedEvent() = default;
	};
}