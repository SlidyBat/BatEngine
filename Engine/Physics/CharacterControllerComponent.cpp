#include "PCH.h"
#include "CharacterControllerComponent.h"

namespace Bat
{
	CharacterControllerComponent::CharacterControllerComponent( const CharacterControllerBoxDesc& desc )
	{
		m_Type = ControllerType::BOX;
		m_Desc.box = desc;
	}

	CharacterControllerComponent::CharacterControllerComponent( const CharacterControllerCapsuleDesc& desc )
	{
		m_Type = ControllerType::CAPSULE;
		m_Desc.cap = desc;
	}

	PhysicsControllerCollisionFlags CharacterControllerComponent::Move( const Vec3& disp, float dt )
	{
		return m_pController->Move( disp, dt );
	}
}
