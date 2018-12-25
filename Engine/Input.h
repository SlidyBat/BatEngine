#pragma once

#include "PCH.h"
#include "Event.h"

namespace Bat
{
	class Input : public EventDispatcher
	{
		friend class Window;
	public:
		Input();
		Input( const Input& src ) = delete;
		Input& operator=( const Input& src ) = delete;
		Input( Input&& donor ) = delete;
		Input& operator=( Input&& donor ) = delete;
	private: // keyboard		
		void OnKeyDown( const size_t key );
		void OnKeyUp( const size_t key );
	public:
		bool IsKeyPressed( const size_t key ) const;
	public: // mouse
		enum class MouseButton
		{
			Left,
			Right,
			Middle,
			X1,
			X2,
			TOTAL_MOUSE_BUTTONS
		};
	private:
		void OnMouseMoved( const Vei2& pos );
		void OnMouseWheelScrolled( const Vei2& pos, const float delta );
		void OnMouseButtonDown( const Vei2& pos, const MouseButton mb );
		void OnMouseButtonUp( const Vei2& pos, const MouseButton mb );
		void OnMouseButtonDblClick( const Vei2& pos, const MouseButton mb );
	public:
		Vei2 GetMousePosition() const;
		bool IsMouseButtonDown( const MouseButton mb ) const;

		bool IsLeftDown() const;
		bool IsRightDown() const;
		bool IsMiddleDown() const;
		bool IsX1Down() const;
		bool IsX2Down() const;
	private:
		static constexpr int MaxKeys = 256;
		bool m_bKeyIsPressed[MaxKeys];

		Vei2 m_vecMousePosition;
		bool m_bMouseButtonIsDown[(int)MouseButton::TOTAL_MOUSE_BUTTONS];
	};
}