#pragma once

#include "Event.h"

namespace Bat
{
	class SceneNode;

	// Called when node is added to the scene
	struct SceneNodeAddedEvent
	{
		SceneNode* node;
	};

	// Called when node is removed from the scene
	struct SceneNodeRemovedEvent
	{
		SceneNode* node;
	};
}