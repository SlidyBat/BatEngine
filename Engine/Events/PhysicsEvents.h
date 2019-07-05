#pragma once

#include "Event.h"

namespace Bat
{
	class IPhysicsObject;

	// Called when a physics object awakes
	struct PhysicsObjectWakeEvent
	{
		IPhysicsObject* object; // Object that awoke
	};

	// Called when a physics object sleeps
	struct PhysicsObjectSleepEvent
	{
		IPhysicsObject* object; // Object that went to sleep
	};

	// Called when two physics objects first start touching
	struct PhysicsObjectStartTouchEvent
	{
		IPhysicsObject* a;
		IPhysicsObject* b;
	};

	// Called while two physics objects are touching
	struct PhysicsObjectTouchEvent
	{
		IPhysicsObject* a;
		IPhysicsObject* b;
	};

	// Called when two physics objects stop touching
	struct PhysicsObjectEndTouchEvent
	{
		IPhysicsObject* a;
		IPhysicsObject* b;
	};

	// Called when an object enters a trigger volume
	struct PhysicsTriggerStartTouchEvent
	{
		IPhysicsObject* trigger;
		IPhysicsObject* other;
	};

	// Called when an object exits a trigger volume
	struct PhysicsTriggerEndTouchEvent
	{
		IPhysicsObject* trigger;
		IPhysicsObject* other;
	};
}