#pragma once

#include <array>
#include <unordered_map>
#include <cstdint>
#include <bitset>
#include <typeindex>

#include "Events/Event.h"
#include "Events/EntityEvents.h"
#include "Util/ChunkedAllocator.h"
#include "Util/Reflect.h"

namespace Bat
{
#define BAT_COMPONENT_LIST(_)   \
	_(TRANSFORM)            \
	_(NAME)                 \
	_(MODEL)                \
	_(LIGHT)                \
	_(PHYSICS)              \
	_(ANIMATION)            \
	_(PARTICLE_EMITTER)     \
	_(CHARACTER_CONTROLLER) \
	_(BEHAVIOUR_TREE)       \

	enum class ComponentId
	{
		INVALID = -1,
#define MAKE_ENUM( component ) component,
		BAT_COMPONENT_LIST( MAKE_ENUM )
#undef MAKE_ENUM
		LAST
	};

#define BAT_COMPONENT( id ) \
	BAT_REFLECT(); \
	static constexpr const char* GetName() { return #id; } \
	static constexpr ComponentId GetId() { return ComponentId::##id; } \
	static constexpr size_t GetIndex() { return (size_t)GetId(); }

#define BAT_COMPONENT_BEGIN( classname ) \
	template <> \
	TypeDescriptor Bat::GetComponentTypeDescriptor<classname::GetId()>() { return classname::Reflect(); } \
	template <> \
	void Bat::Detail::DestroyComponent<classname::GetId()>( void* pComponent ) { reinterpret_cast<classname*>( pComponent )->~classname(); } \
	BAT_REFLECT_BEGIN( classname );

#define BAT_COMPONENT_MEMBER( membername ) \
	BAT_REFLECT_MEMBER( membername )

#define BAT_COMPONENT_END() \
	BAT_REFLECT_END();

	template <ComponentId Id>
	TypeDescriptor GetComponentTypeDescriptor();
	
	TypeDescriptor GetComponentTypeDescriptor( ComponentId id );

	namespace Detail
	{
		template <ComponentId Id>
		void DestroyComponent( void* pComponent );
	}

	class EntityManager;

	class Entity
	{
	public:
		BAT_REFLECT();

		struct Id
		{
		public:
			BAT_REFLECT();

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

	class EntityManager : public EventDispatcher
	{
	public:
		BAT_REFLECT();

		EntityManager();
		
		Entity CreateEntity();
		void DestroyEntity( Entity entity );

		inline bool IsStale( Entity e );

		template <typename C, typename... Args>
		C& AddComponent( Entity entity, Args&&... args );
		template <typename C>
		void RemoveComponent( Entity entity );
		template <typename C>
		C& GetComponent( Entity entity );
		void* GetComponent( Entity entity, ComponentId id );
		template <typename C>
		const C& GetComponent( Entity entity ) const;
		template <typename C>
		bool HasComponent( Entity entity );
		std::vector<ComponentId> GetComponentsList( Entity entity );

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

		void EnsureEntityCapacity( uint32_t index );
	private:
		using ComponentAllocator = ChunkedAllocator<8192>;

		ComponentAllocator& GetComponentAllocator( ComponentId id );
		template <typename C>
		ComponentAllocator& GetComponentAllocator();

		void SortFreeList();
	private:
		static constexpr size_t MAX_COMPONENTS = 100;
		static constexpr size_t INITIAL_ENTITIES = 1000;

		std::vector<uint32_t> m_EntityVersions;
		std::vector<uint32_t> m_FreeList;
		uint32_t m_iEntityHead = 0;

		using ComponentMask = std::bitset<MAX_COMPONENTS>;
		std::vector<ComponentMask> m_EntityComponentMasks;
		std::array<std::unique_ptr<ComponentAllocator>, MAX_COMPONENTS> m_pComponentAllocators;
	};

	template<typename C, typename... Args>
	inline C& EntityManager::AddComponent( Entity entity, Args&&... args )
	{
		const size_t component_idx = C::GetIndex();
		const size_t entity_idx = entity.GetId().GetIndex();
		ASSERT( !m_EntityComponentMasks[entity_idx].test( component_idx ), "Added same component twice" );
		ComponentAllocator& allocator = GetComponentAllocator<C>();

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
		ComponentAllocator& allocator = GetComponentAllocator<C>();

		C* pComponent = reinterpret_cast<C*>( allocator.Get( entity_idx ) );
		DispatchEvent<ComponentRemovedEvent<C>>( entity, *pComponent );
		pComponent->~C();

		m_EntityComponentMasks[entity_idx].reset( component_idx ); // clear component mask
	}

	template<typename C>
	inline C& EntityManager::GetComponent( Entity entity )
	{
		ASSERT( HasComponent<C>( entity ), "Tried to get component we don't have" );
		const size_t entity_idx = entity.GetId().GetIndex();
		ComponentAllocator& allocator = GetComponentAllocator<C>();

		C* pComponent = reinterpret_cast<C*>( allocator.Get( entity_idx ) );
		return *pComponent;
	}

	template<typename C>
	inline const C& EntityManager::GetComponent( Entity entity ) const
	{
		ASSERT( HasComponent<C>( entity ), "Tried to get component we don't have" );
		const size_t entity_idx = entity.GetId().GetIndex();
		ComponentAllocator& allocator = GetComponentAllocator<C>();

		const C* pComponent = reinterpret_cast<const C*>( allocator.Get( entity_idx ) );
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
	inline EntityManager::ComponentAllocator& EntityManager::GetComponentAllocator()
	{
		const ComponentId component_id = C::GetId();
		return GetComponentAllocator( component_id );
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

	extern EntityManager world;
}