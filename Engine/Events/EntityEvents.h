#pragma once

#include "Event.h"

namespace Bat
{
	class Entity;

	struct EntityCreatedEvent
	{
		Entity& entity;
	};

	struct EntityDestroyedEvent
	{
		const Entity& entity;
	};

	template <typename C>
	struct ComponentAddedEvent
	{
		Entity& entity;
		C& component;
	};

	template <typename C>
	struct ComponentRemovedEvent
	{
		Entity& entity;
		const C& component;
	};
}