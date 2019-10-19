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
			const auto& t = e.entity.Get<TransformComponent>();
			if( e.component.GetType() == PhysicsObjectType::STATIC )
			{
				e.component.m_pObject = std::unique_ptr<IPhysicsObject>( Physics::CreateStaticObject( t.GetPosition(), t.GetRotation(), (void*)e.entity.GetId().Raw() ) );
			}
			else if( e.component.GetType() == PhysicsObjectType::DYNAMIC )
			{
				e.component.m_pObject = std::unique_ptr<IPhysicsObject>( Physics::CreateDynamicObject( t.GetPosition(), t.GetRotation(), (void*)e.entity.GetId().Raw() ) );
			}
		}

		void Update( EntityManager& world, float deltatime )
		{
			for( Entity ent : world )
			{
				if( ent.Has<PhysicsComponent>() )
				{
					auto& phys = ent.Get<PhysicsComponent>();
					auto& t = ent.Get<TransformComponent>();
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
									t.GetScale(),
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
									t.GetScale() );
							}
						}
						phys.m_AddMeshShape = PhysicsComponent::AddMeshType::NONE;
					}

					// Kinematic objects update physics system, non-kinematic objects are updated by physics system
					if( phys.GetType() == PhysicsObjectType::DYNAMIC && phys.IsKinematic() )
					{
						static_cast<IDynamicObject*>(obj)->MoveTo( t.GetPosition(), t.GetRotation() );
;					}
					else
					{
						t.SetPosition( obj->GetPosition() );
						t.SetRotation( obj->GetRotation() );
					}

				}
			}
		}
	};
}