#pragma once

#include "Entity.h"
#include "Physics.h"
#include "PhysicsComponent.h"
#include "Model.h"

namespace Bat
{
	class PhysicsSystem
	{
	public:
		PhysicsSystem( EntityManager& world )
		{
			world.AddEventListener<ComponentAddedEvent<PhysicsComponent>>( *this );
		}

		void OnEvent( const ComponentAddedEvent<PhysicsComponent>& e )
		{
			auto& hier = e.entity.Get<HierarchyComponent>();
			if( e.component.GetType() == PhysicsObjectType::STATIC )
			{
				e.component.m_pObject = std::unique_ptr<IPhysicsObject>( Physics::CreateStaticObject( hier.GetAbsPosition(), hier.GetAbsRotation(), (void*)e.entity.GetId().Raw() ) );
			}
			else if( e.component.GetType() == PhysicsObjectType::DYNAMIC )
			{
				e.component.m_pObject = std::unique_ptr<IPhysicsObject>( Physics::CreateDynamicObject( hier.GetAbsPosition(), hier.GetAbsRotation(), (void*)e.entity.GetId().Raw() ) );
			}
		}

		void Update( EntityManager& world, float deltatime )
		{
			for( Entity ent : world )
			{
				if( ent.Has<PhysicsComponent>() )
				{
					auto& phys = ent.Get<PhysicsComponent>();
					auto& hier = ent.Get<HierarchyComponent>();

					IPhysicsObject* obj = phys.m_pObject.get();

					if( phys.m_AddMeshShape != PhysicsComponent::AddMeshType::NONE )
					{
						if( phys.m_AddMeshShape == PhysicsComponent::AddMeshType::SHAPE )
						{
							const auto& model = ent.Get<ModelComponent>();
							for( const auto& mesh : model.GetMeshes() )
							{
								obj->AddMeshShape(
									mesh->GetVertexData(), mesh->GetVertexCount(),
									mesh->GetIndexData(), mesh->GetIndexCount(),
									hier.GetAbsScale(),
									phys.m_Material );
							}
						}
						else if( phys.m_AddMeshShape == PhysicsComponent::AddMeshType::TRIGGER )
						{
							const auto& model = ent.Get<ModelComponent>();
							for( const auto& mesh : model.GetMeshes() )
							{
								obj->AddMeshTrigger(
									mesh->GetVertexData(), mesh->GetVertexCount(),
									mesh->GetIndexData(), mesh->GetIndexCount(),
									hier.GetAbsScale() );
							}
						}
						phys.m_AddMeshShape = PhysicsComponent::AddMeshType::NONE;
					}

					// Kinematic objects update physics system, non-kinematic objects are updated by physics system
					if( phys.GetType() == PhysicsObjectType::DYNAMIC )
					{
						if( phys.IsKinematic() )
						{
							static_cast<IDynamicObject*>( obj )->MoveTo( hier.GetAbsPosition(), hier.GetAbsRotation() );
						}
						else
						{
							hier.SetAbsPosition( obj->GetPosition() );
							hier.SetAbsRotation( obj->GetRotation() );
						}
;					}
				}
			}
		}
	};
}