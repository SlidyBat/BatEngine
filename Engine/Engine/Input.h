#pragma once

class Input
{
public:
	Input();
	Input( const Input& src ) = delete;
	Input& operator=( const Input& src ) = delete;
	Input( Input&& donor ) = delete;
	Input& operator=( Input&& donor ) = delete;

	void KeyDown( const size_t key );
	void KeyUp( const size_t key );

	bool IsKeyPressed( const size_t key ) const;
private:
	static constexpr int MaxKeys = 256;
	bool m_bKeyIsPressed[256];
};