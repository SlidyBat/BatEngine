#pragma once

class Input
{
public:
	Input();
	Input( const Input& src ) = delete;
	Input& operator=( const Input& src ) = delete;
	Input( Input&& donor ) = delete;
	Input& operator=( Input&& donor ) = delete;

	static void KeyDown( const size_t key );
	static void KeyUp( const size_t key );

	static bool IsKeyPressed( const size_t key );
private:
	static Input& Get();

	void _KeyDown( const size_t key );
	void _KeyUp( const size_t key );

	bool _IsKeyPressed( const size_t key ) const;
private:
	static constexpr int MaxKeys = 256;
	bool m_bKeyIsPressed[256];
};