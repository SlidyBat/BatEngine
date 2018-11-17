#pragma once

#include "Camera.h"

namespace Bat
{
	class Input;

	class MoveableCamera : public Camera
	{
	public:
		void Update( const Input& input, float deltatime );
	};
}