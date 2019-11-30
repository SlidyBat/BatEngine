#pragma once

#include "Camera.h"

namespace Bat
{
	class Input;
	struct MouseMovedEvent;

	class MoveableCamera : public Camera
	{
	public:
		MoveableCamera( Input& input, float speed = 20.0f, float angSpeed = 1.0f );

		void Update( float deltatime );
		void OnEvent( const MouseMovedEvent& e );

		float GetSpeed() const { return m_fSpeed; }
		void SetSpeed( const float speed ) { m_fSpeed = speed; }
		float GetAngularSpeed() const { return m_fAngularSpeed; }
		void SetAngularSpeed( const float angspeed ) { m_fAngularSpeed = angspeed; }
	private:
		Input& m_Input;
		float m_fSpeed;
		float m_fAngularSpeed;
	};
}