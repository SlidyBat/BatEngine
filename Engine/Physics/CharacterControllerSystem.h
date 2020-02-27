#pragma once

#include "Entity.h"
#include "Physics.h"
#include "CharacterControllerComponent.h"

namespace Bat
{
	class CharacterControllerSystem
	{
	public:
		CharacterControllerSystem( EntityManager& world )
		{
			world.AddEventListener<ComponentAddedEvent<CharacterControllerComponent>>( *this );
		}

		void OnEvent( const ComponentAddedEvent<CharacterControllerComponent>& e )
		{
			auto& hier = e.entity.Get<HierarchyComponent>();
			if( e.component.m_Type == CharacterControllerComponent::ControllerType::BOX )
			{
				auto& desc = e.component.m_Desc.box;

				PhysicsBoxControllerDesc box_desc;
				box_desc.position = hier.GetAbsPosition();
				box_desc.height = desc.height;
				box_desc.forward_extent = desc.forward_extent;
				box_desc.side_extent = desc.side_extent;
				box_desc.slope_limit = desc.slope_limit;
				box_desc.step_offset = desc.step_offset;
				box_desc.material = desc.material;
				box_desc.user_data = (void*)e.entity.GetId().Raw();
				e.component.m_pController = std::unique_ptr<ICharacterController>( Physics::CreateCharacterController( box_desc ) );
			}
			else if( e.component.m_Type == CharacterControllerComponent::ControllerType::CAPSULE )
			{
				auto& desc = e.component.m_Desc.cap;

				PhysicsCapsuleControllerDesc cap_desc;
				cap_desc.position = hier.GetAbsPosition();
				cap_desc.height = desc.height;
				cap_desc.radius = desc.radius;
				cap_desc.slope_limit = desc.slope_limit;
				cap_desc.step_offset = desc.step_offset;
				cap_desc.material = desc.material;
				cap_desc.user_data = (void*)e.entity.GetId().Raw();
				e.component.m_pController = std::unique_ptr<ICharacterController>( Physics::CreateCharacterController( cap_desc ) );
			}
		}

		void Update( EntityManager& world, float deltatime )
		{
			for( Entity ent : world )
			{
				if( ent.Has<CharacterControllerComponent>() )
				{
					auto& cont = ent.Get<CharacterControllerComponent>();
					auto& hier = ent.Get<HierarchyComponent>();

					hier.SetAbsPosition( cont.m_pController->GetPosition() );
				}
			}
		}
	};
}