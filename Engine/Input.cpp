#include "PCH.h"
#include "Input.h"

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

	void Input::OnKeyDown( const size_t key, bool repeated )
	{
		ASSERT( key >= 0, "Invalid keyboard key '{}'", key );
		ASSERT( key < MaxKeys, "Invalid keyboard key '{}'", key );

		m_bKeyIsPressed[key] = true;

		DispatchEvent<KeyPressedEvent>( key, repeated );
	}

	void Input::OnKeyUp( const size_t key )
	{
		ASSERT( key >= 0, "Invalid keyboard key '{}'", key );
		ASSERT( key < MaxKeys, "Invalid keyboard key '{}'", key );

		m_bKeyIsPressed[key] = false;

		DispatchEvent<KeyReleasedEvent>( key );
	}

	bool Input::IsKeyDown( const size_t key ) const
	{
		return m_bKeyIsPressed[key];
	}

	void Input::OnMouseMoved( const Vei2& pos )
	{
		DispatchEvent<MouseMovedEvent>( pos, m_vecMousePosition );

		m_vecMousePosition = pos;
	}

	void Input::OnMouseWheelScrolled( const Vei2& pos, const float delta )
	{
		m_vecMousePosition = pos;

		DispatchEvent<MouseScrolledEvent>( pos, delta );
	}

	void Input::OnMouseButtonDown( const Vei2& pos, const MouseButton mb )
	{
		m_vecMousePosition = pos;
		m_bMouseButtonIsDown[(int)mb] = true;

		DispatchEvent<MouseButtonPressedEvent>( pos, mb );
	}

	void Input::OnMouseButtonUp( const Vei2& pos, const MouseButton mb )
	{
		m_vecMousePosition = pos;
		m_bMouseButtonIsDown[(int)mb] = false;

		DispatchEvent<MouseButtonReleasedEvent>( pos, mb );
	}

	void Input::OnMouseButtonDblClick( const Vei2& pos, const MouseButton mb )
	{
		m_vecMousePosition = pos;
		m_bMouseButtonIsDown[(int)mb] = true;

		DispatchEvent<MouseButtonDoubleClickEvent>( pos, mb );
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