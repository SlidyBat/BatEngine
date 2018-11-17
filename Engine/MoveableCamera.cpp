#include "MoveableCamera.h"

#include "Input.h"
#include "BatWinAPI.h"

namespace Bat
{
	void MoveableCamera::Update( const Input & input, float deltatime )
	{
		const Vec3 forward = GetForwardVector();
		const Vec3 right = GetRightVector();

		const float speed = 20.0f;
		const float angSpeed = 200.0f;

		if( input.IsKeyPressed( 'A' ) )
		{
			MoveBy( -right * speed * deltatime );
		}
		if( input.IsKeyPressed( 'D' ) )
		{
			MoveBy( right * speed * deltatime );
		}
		if( input.IsKeyPressed( 'W' ) )
		{
			MoveBy( forward * speed * deltatime );
		}
		if( input.IsKeyPressed( 'S' ) )
		{
			MoveBy( -forward * speed * deltatime );
		}

		if( input.IsKeyPressed( VK_UP ) )
		{
			RotateBy( -angSpeed * deltatime, 0.0f, 0.0f );
		}
		if( input.IsKeyPressed( VK_DOWN ) )
		{
			RotateBy( angSpeed * deltatime, 0.0f, 0.0f );
		}
		if( input.IsKeyPressed( VK_LEFT ) )
		{
			RotateBy( 0.0f, -angSpeed * deltatime, 0.0f );
		}
		if( input.IsKeyPressed( VK_RIGHT ) )
		{
			RotateBy( 0.0f, angSpeed * deltatime, 0.0f );
		}

		if( input.IsKeyPressed( VK_SPACE ) )
		{
			MoveBy( 0.0f, speed * deltatime, 0.0f );
		}
		if( input.IsKeyPressed( VK_SHIFT ) )
		{
			MoveBy( 0.0f, -speed * deltatime, 0.0f );
		}
	}
}