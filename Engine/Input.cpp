#include "Input.h"
#include <cassert>

Input::Input()
{
	for( int i = 0; i < MaxKeys; i++ )
	{
		m_bKeyIsPressed[i] = false;
	}
}

void Input::KeyDown( const size_t key )
{
	assert( key >= 0 );
	assert( key < MaxKeys );

	m_bKeyIsPressed[key] = true;
}

void Input::KeyUp( const size_t key )
{
	assert( key >= 0 );
	assert( key < MaxKeys );

	m_bKeyIsPressed[key] = false;
}

bool Input::IsKeyPressed( const size_t key ) const
{
	return m_bKeyIsPressed[key];
}

void Input::MouseButtonDown( const MouseButton mb )
{
	m_bMouseButtonIsDown[(int)mb] = true;
}

void Input::MouseButtonUp( const MouseButton mb )
{
	m_bMouseButtonIsDown[(int)mb] = false;
}

void Input::MouseButtonDblClick( const MouseButton mb )
{
	m_bMouseButtonIsDown[(int)mb] = true; // might do more with double click later, but for now it just means mouse down
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