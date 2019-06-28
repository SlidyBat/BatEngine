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

		const TreeNode* GetParentNode() const { return m_pParentNode; }
		TreeNode* GetParentNode() { return m_pParentNode; }

		size_t GetNumChildNodes() const { return m_ChildNodes.size(); }
		TreeNode& GetChildNode( size_t idx ) { return m_ChildNodes[idx]; }
		const TreeNode& GetChildNode( size_t idx ) const { return m_ChildNodes[idx]; }

		// Creates a child node with the given value and returns it's index
		size_t AddChildNode( T value )
		{
			size_t new_idx = m_ChildNodes.size();
			m_ChildNodes.emplace_back( value, this );
			return new_idx;
		}
		// Removes the child node at the given index
		void RemoveChildNode( size_t idx )
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