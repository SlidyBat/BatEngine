#pragma once

#include "Camera.h"

namespace Bat
{
	class Input;

	class MoveableCamera : public Camera
	{
	public:
		MoveableCamera( float speed = 20.0f, float angSpeed = 200.0f );

		void Update( const Input& input, float deltatime );

		float GetSpeed() const { return m_fSpeed; }
		void SetSpeed( const float speed ) { m_fSpeed = speed; }
		float GetAngularSpeed() const { return m_fAngularSpeed; }
		void SetAngularSpeed( const float angspeed ) { m_fAngularSpeed = angspeed; }
	private:
		float m_fSpeed;
		float m_fAngularSpeed;
	};
}