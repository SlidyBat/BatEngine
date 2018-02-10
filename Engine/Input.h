#pragma once

class Input
{
public:
	Input();
	Input( const Input& src ) = delete;
	Input& operator=( const Input& src ) = delete;
	Input( Input&& donor ) = delete;
	Input& operator=( Input&& donor ) = delete;

	// keyboard
	static void KeyDown( const size_t key );
	static void KeyUp( const size_t key );
	static bool IsKeyPressed( const size_t key );
	// mouse
public:
	enum MouseButton
	{
		Left,
		Right,
		Middle,
		X1,
		X2,
		TOTAL_MOUSE_BUTTONS
	};
public:
	static void MouseButtonDown( const MouseButton mb );
	static void MouseButtonUp( const MouseButton mb );
	static void MouseButtonDblClick( const MouseButton mb );

	static bool IsLeftDown();
	static bool IsRightDown();
	static bool IsMiddleDown();
	static bool IsX1Down(); // are there more than 2 X buttons?
	static bool IsX2Down();
private:
	static Input& Get();

	void _KeyDown( const size_t key );
	void _KeyUp( const size_t key );
	bool _IsKeyPressed( const size_t key ) const;

	void _MouseButtonDown( const MouseButton mb );
	void _MouseButtonUp( const MouseButton mb );
	void _MouseButtonDblClick( const MouseButton mb );
	bool _IsMouseButtonDown( const MouseButton mb ) const;
private:
	static constexpr int MaxKeys = 256;
	bool m_bKeyIsPressed[256];

	bool m_bMouseButtonIsDown[TOTAL_MOUSE_BUTTONS];
};