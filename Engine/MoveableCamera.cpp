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

		if( m_Input.IsKeyPressed( 'A' ) )
		{
			MoveBy( -right * m_fSpeed * deltatime );
		}
		if( m_Input.IsKeyPressed( 'D' ) )
		{
			MoveBy( right * m_fSpeed * deltatime );
		}
		if( m_Input.IsKeyPressed( 'W' ) )
		{
			MoveBy( forward * m_fSpeed * deltatime );
		}
		if( m_Input.IsKeyPressed( 'S' ) )
		{
			MoveBy( -forward * m_fSpeed * deltatime );
		}

		if( m_Input.IsKeyPressed( VK_UP ) )
		{
			RotateBy( -m_fAngularSpeed * deltatime, 0.0f, 0.0f );
		}
		if( m_Input.IsKeyPressed( VK_DOWN ) )
		{
			RotateBy( m_fAngularSpeed * deltatime, 0.0f, 0.0f );
		}
		if( m_Input.IsKeyPressed( VK_LEFT ) )
		{
			RotateBy( 0.0f, -m_fAngularSpeed * deltatime, 0.0f );
		}
		if( m_Input.IsKeyPressed( VK_RIGHT ) )
		{
			RotateBy( 0.0f, m_fAngularSpeed * deltatime, 0.0f );
		}

		if( m_Input.IsKeyPressed( VK_SPACE ) )
		{
			MoveBy( 0.0f, m_fSpeed * deltatime, 0.0f );
		}
		if( m_Input.IsKeyPressed( VK_SHIFT ) )
		{
			MoveBy( 0.0f, -m_fSpeed * deltatime, 0.0f );
		}
	}

	void MoveableCamera::OnEvent( const MouseMovedEvent& e )
	{
		if( m_Input.IsLeftDown() )
		{
			const Vei2& delta = e.GetDeltaPosition();
			const float deltayaw = GetAngularSpeed() * delta.y * g_pGlobals->deltatime;
			const float deltapitch = GetAngularSpeed() * delta.x * g_pGlobals->deltatime;

			RotateBy( deltapitch, deltayaw, 0.0f );
		}
	}
}