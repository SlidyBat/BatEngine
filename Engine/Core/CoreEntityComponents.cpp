#include "PCH.h"
#include "CoreEntityComponents.h"

#include "Scene.h"

namespace Bat
{
	TransformComponent::TransformComponent( SceneNode* node )
		:
		m_pNode( node )
	{
	}
	const Vec3& TransformComponent::GetPosition() const
	{
		if( IsDirty( HierarchyCache::POS ) )
		{
			CalculateCache();
		}
		return m_vecPosition;
	}

	TransformComponent& TransformComponent::SetPosition( float x, float y, float z )
	{
		return SetPosition( { x, y, z } );
	}

	TransformComponent& TransformComponent::SetPosition( const Vec3& pos )
	{
		if( !IsDirty( HierarchyCache::POS ) && m_vecPosition == pos )
		{
			return *this;
		}

		MarkSelfAndChildrenDirty( HierarchyCache::POS );
		ClearDirty( HierarchyCache::POS );

		m_vecPosition = pos;
		m_matLocalToWorld.SetTranslation( pos );

		SceneNode* parent = m_pNode->GetParent();
		if( !parent )
		{
			m_vecLocalPosition = pos;
		}
		else
		{
			Entity parent_ent = parent->Get();
			auto& parent_transform = parent_ent.Get<TransformComponent>();
			Mat4 world_to_parent = Mat4::Inverse( parent_transform.LocalToWorldMatrix() );
			m_vecLocalPosition = world_to_parent * m_vecPosition;
		}

		return *this;
	}

	const Vec3& TransformComponent::GetRotation() const
	{
		if( IsDirty( HierarchyCache::ROT ) )
		{
			CalculateCache();
		}
		return m_vecRotation;
	}

	TransformComponent& TransformComponent::SetRotation( float pitch, float yaw, float roll )
	{
		return SetRotation( { pitch, yaw, roll } );
	}

	TransformComponent& TransformComponent::SetRotation( const Vec3& rot )
	{
		if( !IsDirty( HierarchyCache::ROT ) && m_vecRotation == rot )
		{
			return *this;
		}

		MarkSelfAndChildrenDirty( HierarchyCache::ROT );
		ClearDirty( HierarchyCache::ROT );

		m_vecRotation = rot;
		m_matLocalToWorld = Mat4::Scale( m_flScale ) * Mat4::RotateDeg( m_vecRotation );
		m_matLocalToWorld.SetTranslation( m_vecPosition );

		SceneNode* parent = m_pNode->GetParent();
		if( !parent )
		{
			m_vecLocalRotation = rot;
		}
		else
		{
			Entity parent_ent = parent->Get();
			auto& parent_transform = parent_ent.Get<TransformComponent>();
			Mat4 world_to_parent = Mat4::Inverse( parent_transform.LocalToWorldMatrix() );
			Mat4 local = world_to_parent * m_matLocalToWorld;
			local.DecomposeDeg( nullptr, &m_vecLocalRotation, nullptr );
		}

		return *this;
	}

	float TransformComponent::GetScale() const
	{
		if( IsDirty( HierarchyCache::SCALE ) )
		{
			CalculateCache();
		}
		return m_flScale;
	}

	TransformComponent& TransformComponent::SetLocalPosition( float x, float y, float z )
	{
		return SetLocalPosition( { x, y, z } );
	}

	TransformComponent& TransformComponent::SetLocalPosition( const Vec3& pos )
	{
		if( m_vecLocalPosition == pos )
		{
			return *this;
		}

		MarkSelfAndChildrenDirty( HierarchyCache::POS );
		m_vecLocalPosition = pos;

		return *this;
	}

	TransformComponent& TransformComponent::SetScale( float scale )
	{
		if( !IsDirty( HierarchyCache::SCALE ) && m_flScale == scale )
		{
			return *this;
		}

		MarkSelfAndChildrenDirty( HierarchyCache::SCALE );
		ClearDirty( HierarchyCache::SCALE );

		m_flScale = scale;
		m_matLocalToWorld = Mat4::Scale( m_flScale ) * Mat4::RotateDeg( m_vecRotation );
		m_matLocalToWorld.SetTranslation( m_vecPosition );

		SceneNode* parent = m_pNode->GetParent();
		if( !parent )
		{
			m_flLocalScale = scale;
		}
		else
		{
			Entity parent_ent = parent->Get();
			auto& parent_transform = parent_ent.Get<TransformComponent>();
			Mat4 world_to_parent = Mat4::Inverse( parent_transform.LocalToWorldMatrix() );
			Mat4 local = world_to_parent * m_matLocalToWorld;
			local.DecomposeDeg( nullptr, nullptr, &m_flLocalScale );
		}

		return *this;
	}

	const Vec3& TransformComponent::GetLocalPosition() const
	{
		return m_vecLocalPosition;
	}

	TransformComponent& TransformComponent::SetLocalRotation( float pitch, float yaw, float roll )
	{
		return SetLocalRotation( { pitch, yaw, roll } );
	}

	TransformComponent& TransformComponent::SetLocalRotation( const Vec3& rot )
	{
		if( m_vecLocalRotation == rot )
		{
			return *this;
		}

		m_vecLocalRotation = rot;
		MarkSelfAndChildrenDirty( HierarchyCache::ROT );

		return *this;
	}

	const Vec3& TransformComponent::GetLocalRotation() const
	{
		return m_vecLocalRotation;
	}

	TransformComponent& TransformComponent::SetLocalScale( float uniform_scale )
	{
		if( m_flLocalScale == uniform_scale )
		{
			return *this;
		}

		m_flLocalScale = uniform_scale;
		MarkSelfAndChildrenDirty( HierarchyCache::SCALE );

		return *this;
	}

	float TransformComponent::GetLocalScale() const
	{
		return m_flLocalScale;
	}

	void TransformComponent::SetLocalMatrix( const Mat4& local_matrix )
	{
		MarkSelfAndChildrenDirty( HierarchyCache::ALL );
		local_matrix.DecomposeDeg( &m_vecLocalPosition, &m_vecLocalRotation, &m_flLocalScale );
	}

	const Mat4& TransformComponent::LocalToWorldMatrix() const
	{
		if( IsDirty( HierarchyCache::ALL ) )
		{
			CalculateCache();
		}
		return m_matLocalToWorld;
	}

	Mat4 TransformComponent::WorldToLocalMatrix() const
	{
		return Mat4::Inverse( m_matLocalToWorld );
	}

	void TransformComponent::MarkSelfAndChildrenDirty( HierarchyCache cache_type )
	{
		ASSERT( m_pNode, "No associated scene node" );

		m_Dirty |= cache_type;
		for( SceneNode& child_node : *m_pNode )
		{
			Entity child = child_node.Get();
			child.Get<TransformComponent>().MarkSelfAndChildrenDirty( cache_type );
		}
	}

	void TransformComponent::ClearDirty( HierarchyCache cache_type ) const
	{
		m_Dirty &= ~cache_type;
	}

	bool TransformComponent::IsDirty( HierarchyCache cache_type ) const
	{
		return ( m_Dirty & cache_type ) != HierarchyCache::NONE;
	}

	void TransformComponent::CalculateCache() const
	{
		ClearDirty( HierarchyCache::ALL );

		Entity ent = m_pNode->Get();
		auto& t = ent.Get<TransformComponent>();

		Mat4 local = Mat4::Scale( m_flLocalScale ) *
			Mat4::RotateDeg( m_vecLocalRotation ) *
			Mat4::Translate( m_vecLocalPosition );

		SceneNode* parent = m_pNode->GetParent();
		if( !parent )
		{
			m_vecPosition = m_vecLocalPosition;
			m_vecRotation = m_vecLocalRotation;
			m_flScale = m_flLocalScale;
			m_matLocalToWorld = local;
		}
		else
		{
			Entity parent_ent = parent->Get();
			auto& parent_transform = parent_ent.Get<TransformComponent>();

			m_matLocalToWorld = parent_transform.LocalToWorldMatrix() * local;
			m_matLocalToWorld.DecomposeDeg( &m_vecPosition, &m_vecRotation, &m_flScale );
		}
	}

	HierarchySystem::HierarchySystem()
	{
		EventDispatcher::AddGlobalEventListener<SceneNodeAddedEvent>( *this );
	}

	HierarchySystem::~HierarchySystem()
	{
		EventDispatcher::RemoveGlobalEventListener<SceneNodeAddedEvent>( *this );
	}

	void HierarchySystem::OnEvent( const SceneNodeAddedEvent& e )
	{
		Entity node = e.node->Get();
		auto& t = node.Ensure<TransformComponent>( nullptr );
		t.m_pNode = e.node;
		t.MarkSelfAndChildrenDirty( HierarchyCache::ALL );
	}
}