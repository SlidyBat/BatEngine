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
	Get()._KeyDown( key );
}

void Input::KeyUp( const size_t key )
{
	Get()._KeyUp( key );
}

bool Input::IsKeyPressed( const size_t key )
{
	return Get()._IsKeyPressed( key );
}

Input& Input::Get()
{
	static Input instance;
	return instance;
}

void Input::_KeyDown( const size_t key )
{
	assert( key >= 0 );
	assert( key < MaxKeys );

	m_bKeyIsPressed[key] = true;
}

void Input::_KeyUp( const size_t key )
{
	assert( key >= 0 );
	assert( key < MaxKeys );

	m_bKeyIsPressed[key] = false;
}

bool Input::_IsKeyPressed( const size_t key ) const
{
	return m_bKeyIsPressed[key];
}

void Input::_MouseButtonDown( const MouseButton mb )
{
	m_bMouseButtonIsDown[mb] = true;
}

void Input::_MouseButtonUp( const MouseButton mb )
{
	m_bMouseButtonIsDown[mb] = false;
}

void Input::_MouseButtonDblClick( const MouseButton mb )
{
	m_bMouseButtonIsDown[mb] = true; // might do more with double click later, but for now it just means mouse down
}

bool Input::_IsMouseButtonDown( const MouseButton mb ) const
{
	return m_bMouseButtonIsDown[mb];
}

void Input::MouseButtonDown( const MouseButton mb )
{
	Get()._MouseButtonDown( mb );
}

void Input::MouseButtonUp( const MouseButton mb )
{
	Get()._MouseButtonUp( mb );
}

void Input::MouseButtonDblClick( const MouseButton mb )
{
	Get()._MouseButtonDblClick( mb );
}

bool Input::IsLeftDown()
{
	return Get()._IsMouseButtonDown( MouseButton::Left );
}

bool Input::IsRightDown()
{
	return Get()._IsMouseButtonDown( MouseButton::Right );
}

bool Input::IsMiddleDown()
{
	return Get()._IsMouseButtonDown( MouseButton::Middle );
}

bool Input::IsX1Down()
{
	return Get()._IsMouseButtonDown( MouseButton::X1 );
}

bool Input::IsX2Down()
{
	return Get()._IsMouseButtonDown( MouseButton::X2 );
}
