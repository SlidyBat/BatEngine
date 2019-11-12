#pragma once

#include <cstdint>

#include "Event.h"
#include "EntityEvents.h"
#include "ChunkedAllocator.h"
#include "Tree.h"
#include <bitset>

namespace Bat
{
	class EntityManager;

	class Entity
	{
	public:
		struct Id
		{
		public:
			Id() = default;
			explicit Id( uint64_t id )
				:
				id( id )
			{}
			Id( uint32_t index, uint32_t version )
				:
				id( (uint64_t( version ) << 32) | uint64_t( index ) )
			{}

			uint64_t Raw() const { return id; }
			uint32_t GetIndex() const { return id & 0xFFFFFFFF; }
			uint32_t GetVersion() const { return id >> 32; }

			bool operator==( const Id rhs ) { return id == rhs.id; }
			bool operator!=( const Id rhs ) const { return id != rhs.id; }
			bool operator<( const Id rhs ) const { return id < rhs.id; }
		private:
			uint64_t id = 0;
		};

		static Entity INVALID;


		Entity() = default;
		Entity( EntityManager& manager, const Id id )
			:
			id( id ),
			manager( &manager )
		{}

		template <typename C, typename... Args>
		C& Add( Args&& ... args );
		template <typename C>
		void Remove();
		template <typename C>
		C& Get();
		template <typename C>
		const C& Get() const;
		template <typename C>
		bool Has() const;
		template <typename C, typename... Args>
		C& Ensure( Args&& ... args );

		bool operator==( const Entity rhs ) { return id == rhs.id; }
		bool operator!= (const Entity rhs ) const { return id != rhs.id; }
		bool operator< (const Entity rhs ) const { return id < rhs.id; }

		Id GetId() const { return id; }
	private:
		EntityManager* manager = nullptr;
		Id id;
	};

	class BaseComponent
	{
	public:
		void operator delete( void *p ) { ASSERT( false, "Can't explicitly delete component" ); }
		void operator delete[]( void *p ) { ASSERT( false, "Can't explicitly delete component" ); }
	protected:
		static size_t s_iIndexCounter;
	};

	template <typename Derived>
	class Component : public BaseComponent
	{
	private:
		friend class EntityManager;
		static size_t GetIndex()
		{
			static size_t index = s_iIndexCounter++;
			return index;
		}
	};

	class BaseComponentHelper
	{
	public:
		virtual ~BaseComponentHelper() = default;
		virtual void Destroy( void* pComponent ) = 0;
	};

	template <typename C>
	class ComponentHelper : public BaseComponentHelper
	{
		virtual void Destroy( void* pComponent ) override
		{
			auto pConverted = static_cast<C*>( pComponent );
			pConverted->~C();
		}
	};

	class EntityManager : public EventDispatcher
	{
	public:
		EntityManager() { m_EntityVersions.resize( INITIAL_ENTITIES ); m_EntityComponentMasks.resize( INITIAL_ENTITIES ); }
		
		Entity CreateEntity();
		void DestroyEntity( Entity entity );

		inline bool IsStale( Entity e );

		template <typename C, typename... Args>
		C& AddComponent( Entity entity, Args&&... args );
		template <typename C>
		void RemoveComponent( Entity entity );
		template <typename C>
		C& GetComponent( Entity entity );
		template <typename C>
		const C& GetComponent( Entity entity ) const;
		template <typename C>
		bool HasComponent( Entity entity );

		class Iterator
		{
		public:
			Iterator( EntityManager* manager, uint32_t index )
				:
				m_pManager( manager ),
				m_iIndex( index )
			{
				manager->SortFreeList();
			}

			Iterator& operator++()
			{
				m_iIndex++;
				while( m_iIndex < m_pManager->m_iEntityHead && !IsValid() )
				{
					m_iIndex++;
				}
				return *this;
			}
			bool operator==( const Iterator& rhs ) const
			{
				return m_pManager == rhs.m_pManager && m_iIndex == rhs.m_iIndex;
			}
			bool operator!=( const Iterator& rhs ) const
			{
				return !(*this == rhs);
			}
			Entity operator*()
			{
				return Entity( *m_pManager, Entity::Id( m_iIndex, m_pManager->m_EntityVersions[m_iIndex] ) );
			}
		private:
			bool IsValid()
			{
				const std::vector<uint32_t>& free_list = m_pManager->m_FreeList;
				if( m_iFreeIndex < free_list.size() && m_iIndex == free_list[m_iFreeIndex] )
				{
					m_iFreeIndex++;
					return false;
				}
				return true;
			}
		private:
			friend class EntityManager;

			EntityManager* m_pManager = nullptr;
			uint32_t m_iIndex = 0;
			size_t m_iFreeIndex = 0;
		};

		Iterator begin() { return Iterator( this, 0 ); }
		Iterator end() { return Iterator( this, m_iEntityHead ); }
	private:
		template <typename C>
		ObjectChunkedAllocator<C>& GetComponentAllocator();

		void EnsureEntityCapacity( uint32_t index );

		void SortFreeList();
	private:
		static constexpr size_t MAX_COMPONENTS = 100;
		static constexpr size_t INITIAL_ENTITIES = 1000;

		std::vector<uint32_t> m_EntityVersions;
		std::vector<uint32_t> m_FreeList;
		uint32_t m_iEntityHead = 0;

		using ComponentMask = std::bitset<MAX_COMPONENTS>;
		std::vector<ComponentMask> m_EntityComponentMasks;
		std::array<std::unique_ptr<ChunkedAllocator>, MAX_COMPONENTS> m_pComponentAllocators;
		std::array<std::unique_ptr<BaseComponentHelper>, MAX_COMPONENTS> m_pComponentHelpers;
	};

	template<typename C, typename... Args>
	inline C& EntityManager::AddComponent( Entity entity, Args&&... args )
	{
		const size_t component_idx = C::GetIndex();
		const size_t entity_idx = entity.GetId().GetIndex();
		ASSERT( !m_EntityComponentMasks[entity_idx].test( component_idx ), "Added same component twice" );
		ObjectChunkedAllocator<C>& allocator = GetComponentAllocator<C>();

		void* pAlloc = allocator.Get( entity_idx );
		C* pComponent = new( pAlloc ) C{ std::forward<Args>( args )... };

		m_EntityComponentMasks[entity_idx].set( component_idx );

		DispatchEvent<ComponentAddedEvent<C>>( entity, *pComponent );

		return *pComponent;
	}

	template<typename C>
	inline void EntityManager::RemoveComponent( Entity entity )
	{
		const size_t component_idx = C::GetIndex();
		const size_t entity_idx = entity.GetId().GetIndex();
		ASSERT( m_EntityComponentMasks[entity_idx].test( component_idx ), "Removing non-existent component" );
		ObjectChunkedAllocator<C>& allocator = GetComponentAllocator<C>();

		C* pComponent = (C*)allocator.Get( entity_idx );
		DispatchEvent<ComponentRemovedEvent<C>>( entity, *pComponent );
		pComponent->~C();

		m_EntityComponentMasks[entity_idx].reset( component_idx ); // clear component mask
	}

	template<typename C>
	inline C& EntityManager::GetComponent( Entity entity )
	{
		ASSERT( HasComponent<C>( entity ), "Tried to get component we don't have" );
		const size_t component_idx = C::GetIndex();
		const size_t entity_idx = entity.GetId().GetIndex();
		ObjectChunkedAllocator<C>& allocator = GetComponentAllocator<C>();

		C* pComponent = allocator.Get( entity_idx );
		return *pComponent;
	}

	template<typename C>
	inline const C& EntityManager::GetComponent( Entity entity ) const
	{
		ASSERT( HasComponent<C>( entity ), "Tried to get component we don't have" );
		const size_t component_idx = C::GetIndex();
		const size_t entity_idx = entity.GetId().GetIndex();
		ObjectChunkedAllocator<C>& allocator = GetComponentAllocator<C>();

		C* pComponent = allocator.Get( entity_idx );
		return *pComponent;
	}

	template<typename C>
	inline bool EntityManager::HasComponent( Entity entity )
	{
		const size_t component_idx = C::GetIndex();
		const size_t entity_idx = entity.GetId().GetIndex();
		return m_EntityComponentMasks[entity_idx].test( component_idx );
	}

	template<typename C>
	inline ObjectChunkedAllocator<C>& EntityManager::GetComponentAllocator()
	{
		const size_t component_idx = C::GetIndex();
		if( !m_pComponentAllocators[component_idx] )
		{
			auto allocator = std::make_unique<ObjectChunkedAllocator<C>>();
			allocator->EnsureCapacity( m_iEntityHead );
			m_pComponentAllocators[component_idx] = std::move( allocator );
		}

		if( !m_pComponentHelpers[component_idx] )
		{
			m_pComponentHelpers[component_idx] = std::make_unique<ComponentHelper<C>>();
		}

		return *static_cast<ObjectChunkedAllocator<C>*>( m_pComponentAllocators[component_idx].get() );
	}

	template<typename C, typename ...Args>
	inline C& Entity::Add( Args&& ...args )
	{
		return manager->AddComponent<C>( *this, std::forward<Args>( args )... );
	}

	template<typename C>
	inline void Entity::Remove()
	{
		manager->RemoveComponent<C>( *this );
	}

	template<typename C>
	inline C& Entity::Get()
	{
		return manager->GetComponent<C>( *this );
	}

	template<typename C>
	inline const C& Entity::Get() const
	{
		return manager->GetComponent<C>( *this );
	}

	template<typename C>
	inline bool Entity::Has() const
	{
		return manager->HasComponent<C>( *this );
	}

	template<typename C, typename... Args>
	inline C& Entity::Ensure( Args&& ... args )
	{
		if( Has<C>() )
		{
			return Get<C>();
		}
		return Add<C>( std::forward<Args>( args )... );
	}

	using SceneNode = TreeNode<Entity>;
	extern EntityManager world;
}