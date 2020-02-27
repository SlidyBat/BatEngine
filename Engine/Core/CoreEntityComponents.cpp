#include "PCH.h"
#include "CoreEntityComponents.h"

#include "Scene.h"

namespace Bat
{
	HierarchyComponent::HierarchyComponent( SceneNode* node )
		:
		m_pNode( node )
	{
	}
	Vec3 HierarchyComponent::GetAbsPosition()
	{
		if( IsDirty( HierarchyCache::POS ) )
		{
			CalculateCache();
		}
		return m_vecAbsPosition;
	}

	HierarchyComponent& HierarchyComponent::SetAbsPosition( const Vec3& pos )
	{
		if( !IsDirty( HierarchyCache::POS ) && m_vecAbsPosition == pos )
		{
			return *this;
		}

		ClearDirty( HierarchyCache::POS );
		MarkChildrenDirty( HierarchyCache::POS );

		Entity ent = m_pNode->Get();
		auto& t = ent.Get<TransformComponent>();

		m_vecAbsPosition = pos;
		m_matAbsTransform = Mat4::Scale( m_flAbsScale ) *
			Mat4::RotateDeg( m_vecAbsRotation ) *
			Mat4::Translate( m_vecAbsPosition );

		SceneNode* parent = m_pNode->GetParent();
		if( !parent )
		{
			t.SetPosition( pos );
		}
		else
		{
			Entity parent_ent = parent->Get();
			auto& parent_hier = parent_ent.Get<HierarchyComponent>();
			Mat4 world_to_parent = Mat4::Inverse( parent_hier.GetAbsTransform() );
			t.SetPosition( world_to_parent * m_vecAbsPosition );
		}

		return *this;
	}

	Vec3 HierarchyComponent::GetAbsRotation()
	{
		if( IsDirty( HierarchyCache::ROT ) )
		{
			CalculateCache();
		}
		return m_vecAbsRotation;
	}

	HierarchyComponent& HierarchyComponent::SetAbsRotation( const Vec3& rot )
	{
		if( !IsDirty( HierarchyCache::ROT ) && m_vecAbsRotation == rot )
		{
			return *this;
		}

		ClearDirty( HierarchyCache::ROT );
		MarkChildrenDirty( HierarchyCache::ROT );

		Entity ent = m_pNode->Get();
		auto& t = ent.Get<TransformComponent>();

		m_vecAbsRotation = rot;
		m_matAbsTransform = Mat4::Scale( m_flAbsScale ) *
			Mat4::RotateDeg( m_vecAbsRotation ) *
			Mat4::Translate( m_vecAbsPosition );

		SceneNode* parent = m_pNode->GetParent();
		if( !parent )
		{
			t.SetRotation( rot );
		}
		else
		{
			Entity parent_ent = parent->Get();
			auto& parent_hier = parent_ent.Get<HierarchyComponent>();
			Mat4 world_to_parent = Mat4::Inverse( parent_hier.GetAbsTransform() );
			Mat4 local = world_to_parent * m_matAbsTransform;
			t = TransformComponent( local );
		}

		return *this;
	}

	float HierarchyComponent::GetAbsScale()
	{
		if( IsDirty( HierarchyCache::SCALE ) )
		{
			CalculateCache();
		}
		return m_flAbsScale;
	}

	HierarchyComponent& HierarchyComponent::SetAbsScale( float scale )
	{
		if( !IsDirty( HierarchyCache::SCALE ) && m_flAbsScale == scale )
		{
			return *this;
		}

		ClearDirty( HierarchyCache::SCALE );
		MarkChildrenDirty( HierarchyCache::SCALE );

		Entity ent = m_pNode->Get();
		auto& t = ent.Get<TransformComponent>();

		m_flAbsScale = scale;
		m_matAbsTransform = Mat4::Scale( m_flAbsScale ) *
			Mat4::RotateDeg( m_vecAbsRotation ) *
			Mat4::Translate( m_vecAbsPosition );

		SceneNode* parent = m_pNode->GetParent();
		if( !parent )
		{
			t.SetScale( scale );
		}
		else
		{
			Entity parent_ent = parent->Get();
			auto& parent_hier = parent_ent.Get<HierarchyComponent>();
			Mat4 world_to_parent = Mat4::Inverse( parent_hier.GetAbsTransform() );
			Mat4 local = world_to_parent * m_matAbsTransform;
			t = TransformComponent( local );
		}

		return *this;
	}

	const Mat4& HierarchyComponent::GetAbsTransform()
	{
		if( IsDirty( HierarchyCache::ALL ) )
		{
			CalculateCache();
		}
		return m_matAbsTransform;
	}

	void HierarchyComponent::MarkChildrenDirty( HierarchyCache cache_type )
	{
		ASSERT( m_pNode, "No associated scene node" );

		for( SceneNode& child_node : *m_pNode )
		{
			Entity child = child_node.Get();
			child.Get<HierarchyComponent>().MarkChildrenDirty( cache_type );
		}
	}

	void HierarchyComponent::ClearDirty( HierarchyCache cache_type )
	{
		m_Dirty &= ~cache_type;
	}

	bool HierarchyComponent::IsDirty( HierarchyCache cache_type ) const
	{
		return ( m_Dirty & cache_type ) != HierarchyCache::NONE;
	}

	void HierarchyComponent::CalculateCache()
	{
		ClearDirty( HierarchyCache::ALL );

		Entity ent = m_pNode->Get();
		auto& t = ent.Get<TransformComponent>();

		SceneNode* parent = m_pNode->GetParent();
		if( !parent )
		{
			m_matAbsTransform = t.GetTransform();
			m_vecAbsPosition = t.GetPosition();
			m_vecAbsRotation = t.GetRotation();
			m_flAbsScale = t.GetScale();
		}
		else
		{
			Entity parent_ent = parent->Get();
			auto& parent_hier = parent_ent.Get<HierarchyComponent>();

			m_matAbsTransform = parent_hier.GetAbsTransform() * t.GetTransform();

			DirectX::XMMATRIX transform = DirectX::XMMatrixTranspose(
				DirectX::XMLoadFloat4x4( reinterpret_cast<DirectX::XMFLOAT4X4*>( &m_matAbsTransform ) )
			);

			DirectX::XMVECTOR vscale, vrot, vpos;
			DirectX::XMMatrixDecompose( &vscale, &vrot, &vpos, transform );

			m_vecAbsPosition = vpos;
			m_vecAbsRotation = Math::QuaternionToEulerDeg( vrot );
			DirectX::XMStoreFloat( &m_flAbsScale, vscale );
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
		auto& t = node.Ensure<TransformComponent>();
		if( node.Has<HierarchyComponent>() )
		{
			node.Remove<HierarchyComponent>();
		}
		node.Add<HierarchyComponent>( e.node );
	}
}