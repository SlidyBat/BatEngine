#pragma once

#include "Physics.h"
#include "PhysicsComponent.h"

namespace Bat
{
	class EntityManager;

	class PhysicsSystem
	{
	public:
		PhysicsSystem( EntityManager& world );

		void OnEvent( const ComponentAddedEvent<PhysicsComponent>& e );

		void Update( EntityManager& world, float deltatime );
	};
}