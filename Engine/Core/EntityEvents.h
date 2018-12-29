#pragma once

#include "Event.h"
#include "Entity.h"

namespace Bat
{
	struct EntityCreatedEvent
	{
		Entity& entity;
	};

	struct EntityDestroyedEvent
	{
		const Entity& entity;
	};
}