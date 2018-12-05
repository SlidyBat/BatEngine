#include "Input.h"
#include "BatAssert.h"
#include "MouseEvents.h"
#include "KeyboardEvents.h"

namespace Bat
{
	Input::Input()
	{
		for( int i = 0; i < MaxKeys; i++ )
		{
			m_bKeyIsPressed[i] = false;
		}
	}

	void Input::OnKeyDown( const size_t key )
	{
		ASSERT( key >= 0, "Invalid keyboard key" );
		ASSERT( key < MaxKeys, "Invalid keyboard key" );

		m_bKeyIsPressed[key] = true;

		DISPATCH_EVENT( KeyPressedEvent( key ) );
	}

	void Input::OnKeyUp( const size_t key )
	{
		ASSERT( key >= 0, "Invalid keyboard key" );
		ASSERT( key < MaxKeys, "Invalid keyboard key" );

		m_bKeyIsPressed[key] = false;

		DISPATCH_EVENT( KeyReleasedEvent( key ) );
	}

	bool Input::IsKeyPressed( const size_t key ) const
	{
		return m_bKeyIsPressed[key];
	}

	void Input::OnMouseMoved( const Vei2& pos )
	{
		DISPATCH_EVENT( MouseMovedEvent( pos, m_vecMousePosition ) );

		m_vecMousePosition = pos;
	}

	void Input::OnMouseWheelScrolled( const Vei2& pos, const float delta )
	{
		m_vecMousePosition = pos;

		DISPATCH_EVENT( MouseScrolledEvent( pos, delta ) );
	}

	void Input::OnMouseButtonDown( const Vei2& pos, const MouseButton mb )
	{
		m_vecMousePosition = pos;
		m_bMouseButtonIsDown[(int)mb] = true;

		DISPATCH_EVENT( MouseButtonPressedEvent( pos, mb ) );
	}

	void Input::OnMouseButtonUp( const Vei2& pos, const MouseButton mb )
	{
		m_vecMousePosition = pos;
		m_bMouseButtonIsDown[(int)mb] = false;

		DISPATCH_EVENT( MouseButtonReleasedEvent( pos, mb ) );
	}

	void Input::OnMouseButtonDblClick( const Vei2& pos, const MouseButton mb )
	{
		m_vecMousePosition = pos;
		m_bMouseButtonIsDown[(int)mb] = true;

		DISPATCH_EVENT( MouseButtonDoubleClickEvent( pos, mb ) );
	}

	Vei2 Input::GetMousePosition() const
	{
		return m_vecMousePosition;
	}

	bool Input::IsMouseButtonDown( const MouseButton mb ) const
	{
		return m_bMouseButtonIsDown[(int)mb];
	}

	bool Input::IsLeftDown() const
	{
		return IsMouseButtonDown( MouseButton::Left );
	}

	bool Input::IsRightDown() const
	{
		return IsMouseButtonDown( MouseButton::Right );
	}

	bool Input::IsMiddleDown() const
	{
		return IsMouseButtonDown( MouseButton::Middle );
	}

	bool Input::IsX1Down() const
	{
		return IsMouseButtonDown( MouseButton::X1 );
	}

	bool Input::IsX2Down() const
	{
		return IsMouseButtonDown( MouseButton::X2 );
	}
}