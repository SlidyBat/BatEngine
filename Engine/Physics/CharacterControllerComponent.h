#pragma once

#include "Physics.h"
#include "Core/Entity.h"
#include "Core/CoreEntityComponents.h"

namespace Bat
{
	struct CharacterControllerBoxDesc
	{
		float height = 0.5f, forward_extent = 0.1f, side_extent = 0.1f;
		float slope_limit = 45.0f; // Maximum walkable slop angle in degrees
		float step_offset = 0.1f;
		PhysicsMaterial material = Physics::DEFAULT_MATERIAL;
	};

	struct CharacterControllerCapsuleDesc
	{
		float height = 0.5f, radius = 0.1f;
		float slope_limit = 45.0f; // Maximum walkable slop angle in degrees
		float step_offset = 0.1f;
		PhysicsMaterial material = Physics::DEFAULT_MATERIAL;
	};

	class CharacterControllerComponent
	{
	public:
		BAT_COMPONENT( CHARACTER_CONTROLLER );

		CharacterControllerComponent( const CharacterControllerBoxDesc& desc );
		CharacterControllerComponent( const CharacterControllerCapsuleDesc& desc );

		// Moves the character by the given displacement vector
		PhysicsControllerCollisionFlags Move( const Vec3& disp, float dt );
	private:
		friend class CharacterControllerSystem;

		enum class ControllerType
		{
			BOX,
			CAPSULE
		};
		ControllerType m_Type;
		union ControllerDesc
		{
			ControllerDesc()
			{}

			CharacterControllerBoxDesc box;
			CharacterControllerCapsuleDesc cap;
		};
		ControllerDesc m_Desc;
		std::unique_ptr<ICharacterController> m_pController;
	};
}