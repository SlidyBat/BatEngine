#include "PCH.h"
#include "IRenderPass.h"

#include "CoreEntityComponents.h"
#include "Camera.h"
#include "Scene.h"

namespace Bat
{
	const std::vector<RenderNode>& BaseRenderPass::GetNodes() const
	{
		return m_vNodes;
	}

	NodeType BaseRenderPass::GetNodeType( const std::string& name ) const
	{
		for( const RenderNode& node : m_vNodes )
		{
			if( node.name == name )
			{
				return node.type;
			}
		}
		return NodeType::INVALID;
	}

	NodeDataType BaseRenderPass::GetNodeDataType( const std::string & name ) const
	{
		for( const RenderNode& node : m_vNodes )
		{
			if( node.name == name )
			{
				return node.datatype;
			}
		}
		return NodeDataType::INVALID;
	}

	void BaseRenderPass::AddRenderNode( std::string name, NodeType type, NodeDataType datatype )
	{
		RenderNode node;
		node.name = std::move( name );
		node.type = type;
		node.datatype = datatype;

		m_vNodes.emplace_back( node );
	}

	void BaseRenderPass::Traverse()
	{
		for( Entity e : world )
		{
			if( e.Has<HierarchyComponent>() )
			{
				auto& hier = e.Get<HierarchyComponent>();
				Visit( hier.GetAbsTransform(), e );
			}
		}
	}
}
