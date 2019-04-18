#include "PCH.h"
#include "Input.h"

#include <imgui.h>
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

	void Input::OnKeyChar( const size_t key, bool repeated )
	{
		ASSERT( key >= 0, "Invalid keyboard key '%i'", key );
		ASSERT( key < MaxKeys, "Invalid keyboard key '%i'", key );

		ImGuiIO& io = ImGui::GetIO();
		if( io.WantCaptureKeyboard )
		{
			return;
		}

		DispatchEvent<KeyCharEvent>( key, repeated );
	}

	void Input::OnKeyDown( const size_t key, bool repeated )
	{
		if( repeated && !IsAutorepeatEnabled() )
		{
			// we don't want key down messages for autorepeat
			return;
		}

		ASSERT( key >= 0, "Invalid keyboard key '%i'", key );
		ASSERT( key < MaxKeys, "Invalid keyboard key '%i'", key );

		ImGuiIO& io = ImGui::GetIO();
		if( io.WantCaptureKeyboard )
		{
			return;
		}

		m_bKeyIsPressed[key] = true;

		DispatchEvent<KeyPressedEvent>( key, repeated );
	}

	void Input::OnKeyUp( const size_t key )
	{
		ASSERT( key >= 0, "Invalid keyboard key '%i'", key );
		ASSERT( key < MaxKeys, "Invalid keyboard key '%i'", key );

		ImGuiIO& io = ImGui::GetIO();
		if( io.WantCaptureKeyboard )
		{
			return;
		}

		m_bKeyIsPressed[key] = false;

		DispatchEvent<KeyReleasedEvent>( key );
	}

	bool Input::IsKeyDown( const size_t key ) const
	{
		return m_bKeyIsPressed[key];
	}

	void Input::OnMouseMoved( const Vei2& pos )
	{
		ImGuiIO& io = ImGui::GetIO();
		if( io.WantCaptureMouse )
		{
			return;
		}

		DispatchEvent<MouseMovedEvent>( pos, m_vecMousePosition );

		m_vecMousePosition = pos;
	}

	void Input::OnMouseWheelScrolled( const Vei2& pos, const float delta )
	{
		ImGuiIO& io = ImGui::GetIO();
		if( io.WantCaptureMouse )
		{
			return;
		}

		m_vecMousePosition = pos;

		DispatchEvent<MouseScrolledEvent>( pos, delta );
	}

	void Input::OnMouseButtonDown( const Vei2& pos, const MouseButton mb )
	{
		ImGuiIO& io = ImGui::GetIO();
		if( io.WantCaptureMouse )
		{
			return;
		}

		m_vecMousePosition = pos;
		m_bMouseButtonIsDown[(int)mb] = true;

		DispatchEvent<MouseButtonPressedEvent>( pos, mb );
	}

	void Input::OnMouseButtonUp( const Vei2& pos, const MouseButton mb )
	{
		ImGuiIO& io = ImGui::GetIO();
		if( io.WantCaptureMouse )
		{
			return;
		}

		m_vecMousePosition = pos;
		m_bMouseButtonIsDown[(int)mb] = false;

		DispatchEvent<MouseButtonReleasedEvent>( pos, mb );
	}

	void Input::OnMouseButtonDblClick( const Vei2& pos, const MouseButton mb )
	{
		ImGuiIO& io = ImGui::GetIO();
		if( io.WantCaptureMouse )
		{
			return;
		}

		m_vecMousePosition = pos;
		m_bMouseButtonIsDown[(int)mb] = true;

		DispatchEvent<MouseButtonDoubleClickEvent>( pos, mb );
	}

	void Input::OnMouseEnter()
	{
		m_bMouseInWindow = true;

		DispatchEvent<MouseEnterEvent>();
	}

	void Input::OnMouseLeave()
	{
		m_bMouseInWindow = false;

		DispatchEvent<MouseLeaveEvent>();
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

	bool Input::IsMouseInWindow() const
	{
		return m_bMouseInWindow;
	}
}