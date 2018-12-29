#include "PCH.h"
#include "Entity.h"

#include "EntityEvents.h"

namespace Bat
{
	size_t BaseComponent::s_iIndexCounter = 0;

	Entity EntityManager::CreateEntity()
	{
		uint32_t idx;
		uint32_t version = 0;
		if( m_FreeList.empty() )
		{
			idx = m_iEntityHead++;
		}
		else
		{
			idx = m_FreeList.back();
			m_FreeList.pop_back();
			version = m_EntityVersions[idx];
		}

		Entity::Id id( idx, version );
		Entity entity( id );

		DispatchEvent<EntityCreatedEvent>( entity );

		return entity;
	}

	void EntityManager::DestroyEntity( Entity entity )
	{
		DispatchEvent<EntityDestroyedEvent>( entity );

		const size_t entity_idx = entity.GetId().GetIndex();

		for( size_t i = 0; i < MAX_COMPONENTS; i++ )
		{
			auto& helper = m_pComponentHelpers[i];
			if( helper && m_EntityComponentMasks[entity_idx].test( i ) )
			{
				auto& allocator = m_pComponentAllocators[i];
				void* pComponent = allocator->Get( entity_idx );
				helper->Destroy( pComponent );
			}
		}

		m_EntityComponentMasks[entity_idx].reset();
		m_EntityVersions[entity_idx]++; // update version so all other entities get invalidated
	}

	bool EntityManager::IsStale( Entity e )
	{
		return e.GetId().GetVersion() != m_EntityVersions[e.GetId().GetIndex()];
	}

	void EntityManager::EnsureEntityCapacity( uint32_t index )
	{
		const uint32_t capacity = index + 1;
		m_EntityVersions.resize( capacity );
		m_FreeList.resize( capacity );
		m_EntityComponentMasks.resize( capacity );

		for( auto& allocator : m_pComponentAllocators )
		{
			allocator->EnsureCapacity( capacity );
		}
	}
}
