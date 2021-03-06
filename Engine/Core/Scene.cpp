#include "PCH.h"
#include "Scene.h"

#include "Events/Event.h"
#include "Events/SceneEvents.h"

namespace Bat
{
	BAT_REFLECT_BEGIN( SceneNode );
		BAT_REFLECT_MEMBER( m_Value );
	BAT_REFLECT_END();

	SceneNode::SceneNode( Entity value, SceneNode* parent )
		:
		m_Value( value ),
		m_pParentNode( parent )
	{}
	SceneNode::~SceneNode()
	{
		for( SceneNode* child = GetFirstChild(); child != nullptr; )
		{
			SceneNode* temp = child;
			child = child->GetNextChild();
			delete temp;
		}
	}
	SceneNode::SceneNode( SceneNode&& donor )
	{
		*this = std::move( donor );
	}
	SceneNode& SceneNode::operator=( SceneNode&& donor )
	{
		std::swap( m_pParentNode, donor.m_pParentNode );
		std::swap( m_pFirstChild, donor.m_pFirstChild );
		std::swap( m_pAdjacentChild, donor.m_pAdjacentChild );
		std::swap( m_Value, donor.m_Value );

		for( SceneNode* child = m_pFirstChild; child != nullptr; child = child->m_pAdjacentChild )
		{
			child->m_pParentNode = this;
		}
		for( SceneNode* child = donor.m_pFirstChild; child != nullptr; child = child->m_pAdjacentChild )
		{
			child->m_pParentNode = &donor;
		}

		return *this;
	}
	const SceneNode* SceneNode::GetParent() const
	{
		return m_pParentNode;
	}
	SceneNode* SceneNode::GetParent()
	{
		return m_pParentNode;
	}
	SceneNode* SceneNode::GetFirstChild()
	{
		return m_pFirstChild;
	}
	const SceneNode* SceneNode::GetFirstChild() const
	{
		return m_pFirstChild;
	}
	SceneNode* SceneNode::GetNextChild()
	{
		return m_pAdjacentChild;
	}
	const SceneNode* SceneNode::GetNextChild() const
	{
		return m_pAdjacentChild;
	}
	SceneNode* SceneNode::AddChild( SceneNode&& node )
	{
		ASSERT( !node.GetParent(), "Cannot add a child that already has a parent" );
		ASSERT( !node.GetNextChild(), "Cannot add a child that already has adjacent children" );

		SceneNode* new_node = new SceneNode( std::move( node ) );
		new_node->m_pParentNode = this;
		new_node->m_pAdjacentChild = m_pFirstChild;

		m_pFirstChild = new_node;

		EventDispatcher::DispatchGlobalEvent<SceneNodeAddedEvent>( new_node );

		return new_node;
	}
	SceneNode* SceneNode::AddChild( Entity value )
	{
		SceneNode* new_node = new SceneNode;
		new_node->m_pParentNode = this;
		new_node->m_pAdjacentChild = m_pFirstChild;
		new_node->m_Value = value;

		m_pFirstChild = new_node;

		EventDispatcher::DispatchGlobalEvent<SceneNodeAddedEvent>( new_node );

		return new_node;
	}
	void SceneNode::RemoveChild( SceneNode* target )
	{
		if( m_pFirstChild == target )
		{
			EventDispatcher::DispatchGlobalEvent<SceneNodeRemovedEvent>( m_pFirstChild );

			SceneNode* child = m_pFirstChild;
			m_pFirstChild = child->m_pAdjacentChild;
			delete child;
			return;
		}

		SceneNode* prev = m_pFirstChild;
		for( SceneNode* child = m_pFirstChild->GetNextChild(); child != nullptr; child = child->GetNextChild() )
		{
			if( child == target )
			{
				EventDispatcher::DispatchGlobalEvent<SceneNodeRemovedEvent>( child );

				prev->m_pAdjacentChild = child->m_pAdjacentChild;
				delete child;
				return;
			}
		}
	}
	void SceneNode::RemoveChild( Entity value )
	{
		if( m_pFirstChild->Get() == value )
		{
			EventDispatcher::DispatchGlobalEvent<SceneNodeRemovedEvent>( m_pFirstChild );

			SceneNode* child = m_pFirstChild;
			m_pFirstChild = child->m_pAdjacentChild;
			delete child;
			return;
		}

		SceneNode* prev = m_pFirstChild;
		for( SceneNode* child = m_pFirstChild->GetNextChild(); child != nullptr; child = child->GetNextChild() )
		{
			if( child->Get() == value )
			{
				EventDispatcher::DispatchGlobalEvent<SceneNodeRemovedEvent>( child );

				prev->m_pAdjacentChild = child->m_pAdjacentChild;
				delete child;
				return;
			}
		}
	}
	Entity& SceneNode::Get()
	{
		return m_Value;
	}
	const Entity& SceneNode::Get() const
	{
		return m_Value;
	}
	void SceneNode::Set( Entity value )
	{
		m_Value = value;
		EventDispatcher::DispatchGlobalEvent<SceneNodeAddedEvent>( this );
	}
}
