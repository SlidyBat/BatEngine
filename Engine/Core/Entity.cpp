#include "PCH.h"
#include "Entity.h"

namespace Bat
{
	EntityManager world;

	Entity Entity::INVALID = Entity();

	BAT_REFLECT_BEGIN( Entity::Id );
		BAT_REFLECT_MEMBER( id );
	BAT_REFLECT_END();

	BAT_REFLECT_BEGIN( Entity );
		BAT_REFLECT_MEMBER( id );
	BAT_REFLECT_END();

	BAT_REFLECT_BEGIN( EntityManager );
	BAT_REFLECT_END();

	Entity EntityManager::CreateEntity()
	{
		uint32_t idx;
		uint32_t version = 0;
		if( m_FreeList.empty() )
		{
			idx = m_iEntityHead++;
			EnsureEntityCapacity( idx );
		}
		else
		{
			idx = m_FreeList.back();
			m_FreeList.pop_back();
			version = m_EntityVersions[idx];
		}

		Entity::Id id( idx, version );
		Entity entity( *this, id );

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

	void* EntityManager::GetComponent( Entity entity, ComponentId id )
	{
		const size_t entity_idx = entity.GetId().GetIndex();
		auto& allocator = GetComponentAllocator( id );

		return allocator.Get( entity_idx );
	}

	std::vector<ComponentId> EntityManager::GetComponentsList( Entity entity )
	{
		const size_t entity_idx = entity.GetId().GetIndex();
		const ComponentMask& mask = m_EntityComponentMasks[entity_idx];

		std::vector<ComponentId> components;
		for( int i = (int)ComponentId::INVALID + 1; i < (int)ComponentId::LAST; i++ )
		{
			if( mask.test( i ) )
			{
				components.push_back( (ComponentId)i );
			}
		}
		return components;
	}

	void EntityManager::EnsureEntityCapacity( uint32_t index )
	{
		const uint32_t capacity = index + 1;
		if( m_EntityVersions.size() < capacity )
		{
			m_EntityVersions.resize( capacity );
			m_EntityComponentMasks.resize( capacity );

			for( auto& allocator : m_pComponentAllocators )
			{
				if( allocator )
				{
					allocator->EnsureCapacity( capacity );
				}
			}
		}
	}
	ChunkedAllocator<8192>& EntityManager::GetComponentAllocator( ComponentId id )
	{
		const size_t component_idx = (size_t)id;
		ASSERT( m_pComponentAllocators[component_idx], "Tried to get non-existent component allocator" );
		return *m_pComponentAllocators[component_idx].get();
	}

	void EntityManager::SortFreeList()
	{
		std::sort( m_FreeList.begin(), m_FreeList.end() );
	}

	TypeDescriptor GetComponentTypeDescriptor( ComponentId id )
	{
		switch( id )
		{
#define GET_TYPE_DESCRIPTOR( component ) case ComponentId::component: return GetComponentTypeDescriptor<ComponentId::component>();
			BAT_COMPONENT_LIST( GET_TYPE_DESCRIPTOR )
#undef GET_TYPE_DESCRIPTOR
		default:
			ASSERT( false, "Unhandled component type" );
			return {};
		}
	}
}
