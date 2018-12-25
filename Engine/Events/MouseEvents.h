#pragma once

#include "Event.h"
#include "MathLib.h"
#include "Input.h"

namespace Bat
{
	class MouseMovedEvent : public IEvent<MouseMovedEvent>
	{
	public:
		MouseMovedEvent( const Vei2& pos, const Vei2& oldpos )
			:
			m_vecPos( pos ),
			m_vecOldPos( oldpos )
		{}

		Vei2 GetPosition() const { return m_vecPos; }
		Vei2 GetOldPosition() const { return m_vecOldPos; }
		Vei2 GetDeltaPosition() const { return m_vecPos - m_vecOldPos; }
	private:
		Vei2 m_vecPos;
		Vei2 m_vecOldPos;
	};

	class MouseScrolledEvent : public IEvent<MouseScrolledEvent>
	{
	public:
		MouseScrolledEvent( const Vei2& pos, const float delta )
			:
			m_vecPos( pos ),
			m_fDelta( delta )
		{}

		Vei2 GetPosition() const { return m_vecPos; }
		float GetDelta() const { return m_fDelta; }
	private:
		Vei2 m_vecPos;
		float m_fDelta;
	};

	class MouseButtonPressedEvent : public IEvent<MouseButtonPressedEvent>
	{
	public:
		MouseButtonPressedEvent( const Vei2& pos, const Input::MouseButton button )
			:
			m_vecPos( pos ),
			m_Button( button )
		{}

		Vei2 GetPosition() const { return m_vecPos; }
		Input::MouseButton GetButton() const { return m_Button; }
	private:
		Vei2 m_vecPos;
		Input::MouseButton m_Button;
	};

	class MouseButtonReleasedEvent : public IEvent<MouseButtonReleasedEvent>
	{
	public:
		MouseButtonReleasedEvent( const Vei2& pos, const Input::MouseButton button )
			:
			m_vecPos( pos ),
			m_Button( button )
		{}

		Vei2 GetPosition() const { return m_vecPos; }
		Input::MouseButton GetButton() const { return m_Button; }
	private:
		Vei2 m_vecPos;
		Input::MouseButton m_Button;
	};

	class MouseButtonDoubleClickEvent : public IEvent<MouseButtonDoubleClickEvent>
	{
	public:
		MouseButtonDoubleClickEvent( const Vei2& pos, const Input::MouseButton button )
			:
			m_vecPos( pos ),
			m_Button( button )
		{}

		Vei2 GetPosition() const { return m_vecPos; }
		Input::MouseButton GetButton() const { return m_Button; }
	private:
		Vei2 m_vecPos;
		Input::MouseButton m_Button;
	};
}