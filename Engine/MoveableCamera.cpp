#include "BatWinAPI.h"
#include "MoveableCamera.h"

#include "Input.h"

namespace Bat
{
	MoveableCamera::MoveableCamera( float speed, float angSpeed )
		:
		m_fSpeed( speed ),
		m_fAngularSpeed( angSpeed )
	{}

	void MoveableCamera::Update( const Input & input, float deltatime )
	{
		const Vec3 forward = GetForwardVector();
		const Vec3 right = GetRightVector();

		if( input.IsKeyPressed( 'A' ) )
		{
			MoveBy( -right * m_fSpeed * deltatime );
		}
		if( input.IsKeyPressed( 'D' ) )
		{
			MoveBy( right * m_fSpeed * deltatime );
		}
		if( input.IsKeyPressed( 'W' ) )
		{
			MoveBy( forward * m_fSpeed * deltatime );
		}
		if( input.IsKeyPressed( 'S' ) )
		{
			MoveBy( -forward * m_fSpeed * deltatime );
		}

		if( input.IsKeyPressed( VK_UP ) )
		{
			RotateBy( -m_fAngularSpeed * deltatime, 0.0f, 0.0f );
		}
		if( input.IsKeyPressed( VK_DOWN ) )
		{
			RotateBy( m_fAngularSpeed * deltatime, 0.0f, 0.0f );
		}
		if( input.IsKeyPressed( VK_LEFT ) )
		{
			RotateBy( 0.0f, -m_fAngularSpeed * deltatime, 0.0f );
		}
		if( input.IsKeyPressed( VK_RIGHT ) )
		{
			RotateBy( 0.0f, m_fAngularSpeed * deltatime, 0.0f );
		}

		if( input.IsKeyPressed( VK_SPACE ) )
		{
			MoveBy( 0.0f, m_fSpeed * deltatime, 0.0f );
		}
		if( input.IsKeyPressed( VK_SHIFT ) )
		{
			MoveBy( 0.0f, -m_fSpeed * deltatime, 0.0f );
		}
	}
}