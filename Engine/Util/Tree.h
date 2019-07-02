#pragma once

#include <vector>

namespace Bat
{
	template <typename T>
	class TreeNode
	{
	public:
		TreeNode() = default;
		TreeNode( T value, TreeNode* parent = nullptr )
			:
			m_Value( value ),
			m_pParentNode( parent )
		{}

		const TreeNode* GetParent() const { return m_pParentNode; }
		TreeNode* GetParent() { return m_pParentNode; }

		size_t GetNumChildren() const { return m_ChildNodes.size(); }
		TreeNode& GetChild( size_t idx ) { return m_ChildNodes[idx]; }
		const TreeNode& GetChild( size_t idx ) const { return m_ChildNodes[idx]; }

		size_t AddChild( TreeNode node )
		{
			size_t new_idx = m_ChildNodes.size();
			node.m_pParentNode = this;
			m_ChildNodes.emplace_back( std::move( node ) );
			return new_idx;
		}
		// Creates a child node with the given value and returns it's index
		size_t AddChild( T value )
		{
			size_t new_idx = m_ChildNodes.size();
			m_ChildNodes.emplace_back( value, this );
			return new_idx;
		}
		// Removes the child node at the given index
		void RemoveChild( size_t idx )
		{
			m_ChildNodes.erase( m_ChildNodes.begin() + idx );
		}

		T& Get() { return m_Value; }
		const T& Get() const { return m_Value; }
		void Set( T value ) { m_Value = value; }
	private:
		TreeNode* m_pParentNode = nullptr;
		std::vector<TreeNode> m_ChildNodes;
		T m_Value;
	};
}