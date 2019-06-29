#include "PCH.h"
#include "IRenderPass.h"

#include "CoreEntityComponents.h"
#include "Camera.h"

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

	void BaseRenderPass::Traverse( const SceneNode& scene )
	{
		std::stack<const SceneNode*> stack;
		std::stack<DirectX::XMMATRIX> transforms;

		stack.push( &scene );

		DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();
		if( scene.Get().Has<TransformComponent>() )
		{
			transform = scene.Get().Get<TransformComponent>().GetTransform();
		}
		transforms.push( transform );

		while( !stack.empty() )
		{
			const SceneNode* node = stack.top();
			stack.pop();
			transform = transforms.top();
			transforms.pop();

			Entity e = node->Get();

			Visit( transform, *node );

			size_t num_children = node->GetNumChildNodes();
			for( size_t i = 0; i < num_children; i++ )
			{
				stack.push( &node->GetChildNode( i ) );

				Entity child = node->GetChildNode( i ).Get();

				if( child.Has<TransformComponent>() )
				{
					transforms.push( child.Get<TransformComponent>().GetTransform() * transform );
				}
				else
				{
					transforms.push( transform );
				}
			}
		}
	}
}
