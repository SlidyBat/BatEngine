#include "PCH.h"
#include "MoveableCamera.h"

#include "MouseEvents.h"
#include "Input.h"
#include "Globals.h"

namespace Bat
{
	MoveableCamera::MoveableCamera( Input& input, float speed, float angSpeed )
		:
		m_Input( input ),
		m_fSpeed( speed ),
		m_fAngularSpeed( angSpeed )
	{
		input.AddEventListener<MouseMovedEvent>( *this );
	}

	void MoveableCamera::Update( float deltatime )
	{
		const Vec3 forward = GetForwardVector();
		const Vec3 right = GetRightVector();

		if( m_Input.IsKeyDown( 'A' ) )
		{
			MoveBy( -right * m_fSpeed * deltatime );
		}
		if( m_Input.IsKeyDown( 'D' ) )
		{
			MoveBy( right * m_fSpeed * deltatime );
		}
		if( m_Input.IsKeyDown( 'W' ) )
		{
			MoveBy( forward * m_fSpeed * deltatime );
		}
		if( m_Input.IsKeyDown( 'S' ) )
		{
			MoveBy( -forward * m_fSpeed * deltatime );
		}

		if( m_Input.IsKeyDown( VK_UP ) )
		{
			RotateBy( -m_fAngularSpeed, 0.0f, 0.0f );
		}
		if( m_Input.IsKeyDown( VK_DOWN ) )
		{
			RotateBy( m_fAngularSpeed, 0.0f, 0.0f );
		}
		if( m_Input.IsKeyDown( VK_LEFT ) )
		{
			RotateBy( 0.0f, -m_fAngularSpeed, 0.0f );
		}
		if( m_Input.IsKeyDown( VK_RIGHT ) )
		{
			RotateBy( 0.0f, m_fAngularSpeed, 0.0f );
		}

		if( m_Input.IsKeyDown( VK_SPACE ) )
		{
			MoveBy( 0.0f, m_fSpeed * deltatime, 0.0f );
		}
		if( m_Input.IsKeyDown( VK_SHIFT ) )
		{
			MoveBy( 0.0f, -m_fSpeed * deltatime, 0.0f );
		}
	}

	void MoveableCamera::OnEvent( const MouseMovedEvent& e )
	{
		if( m_Input.IsLeftDown() )
		{
			const Vei2& delta = e.GetDeltaPosition();
			const float deltayaw = GetAngularSpeed() * delta.x;
			const float deltapitch = GetAngularSpeed() * delta.y;

			RotateBy( deltapitch, deltayaw, 0.0f );
		}
	}
}