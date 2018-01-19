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
