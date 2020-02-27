#pragma once

#include <vector>
#include "Entity.h"

namespace Bat
{
	class SceneNode
	{
	public:
		SceneNode() = default;
		SceneNode( Entity value, SceneNode* parent = nullptr );
		~SceneNode();
		SceneNode( const SceneNode& ) = delete;
		SceneNode& operator=( const SceneNode& ) = delete;
		SceneNode( SceneNode&& donor );
		SceneNode& operator=( SceneNode&& donor );

		const SceneNode* GetParent() const;
		SceneNode* GetParent();

		SceneNode* GetFirstChild();
		const SceneNode* GetFirstChild() const;
		SceneNode* GetNextChild();
		const SceneNode* GetNextChild() const;

		SceneNode* AddChild( SceneNode&& node );
		// Creates a child node with the given value and returns it's index
		SceneNode* AddChild( Entity value );
		// Removes the child node at the given index
		void RemoveChild( SceneNode* child );
		// Removes child node that holds given value
		void RemoveChild( Entity value );

		Entity& Get();
		const Entity& Get() const;
		void Set( Entity value );

		class Iterator
		{
		public:
			Iterator( SceneNode* node )
				:
				m_pNode( node )
			{}

			Iterator& operator++()
			{
				m_pNode = m_pNode->GetNextChild();
				return *this;
			}
			bool operator==( const Iterator& rhs ) const
			{
				return m_pNode == rhs.m_pNode;
			}
			bool operator!=( const Iterator& rhs ) const
			{
				return !( *this == rhs );
			}
			SceneNode& operator*()
			{
				return *m_pNode;
			}
			const SceneNode& operator*() const
			{
				return *m_pNode;
			}
			SceneNode& operator->()
			{
				return *m_pNode;
			}
			const SceneNode& operator->() const
			{
				return *m_pNode;
			}
		private:
			SceneNode* m_pNode;
		};

		Iterator begin() { return Iterator( m_pFirstChild ); }
		Iterator end() { return Iterator( nullptr ); }
	private:
		SceneNode* m_pParentNode = nullptr;
		SceneNode* m_pFirstChild = nullptr;
		SceneNode* m_pAdjacentChild = nullptr;
		Entity m_Value;
	};
}