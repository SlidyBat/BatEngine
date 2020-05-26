#pragma once

#include "Core/Entity.h"

namespace Bat
{
	class AnimationSystem
	{
	public:
		void Update( EntityManager& world, float dt );
	};
}