#pragma once

namespace Bat
{
	class Input
	{
	public:
		Input();
		Input( const Input& src ) = delete;
		Input& operator=( const Input& src ) = delete;
		Input( Input&& donor ) = delete;
		Input& operator=( Input&& donor ) = delete;

		// keyboard
		void KeyDown( const size_t key );
		void KeyUp( const size_t key );
		bool IsKeyPressed( const size_t key ) const;
		// mouse
	public:
		enum class MouseButton
		{
			Left,
			Right,
			Middle,
			X1,
			X2,
			TOTAL_MOUSE_BUTTONS
		};
	public:
		void MouseButtonDown( const MouseButton mb );
		void MouseButtonUp( const MouseButton mb );
		void MouseButtonDblClick( const MouseButton mb );
		bool IsMouseButtonDown( const MouseButton mb ) const;

		bool IsLeftDown() const;
		bool IsRightDown() const;
		bool IsMiddleDown() const;
		bool IsX1Down() const;
		bool IsX2Down() const;
	private:
		static constexpr int MaxKeys = 256;
		bool m_bKeyIsPressed[MaxKeys];

		bool m_bMouseButtonIsDown[(int)MouseButton::TOTAL_MOUSE_BUTTONS];
	};
}