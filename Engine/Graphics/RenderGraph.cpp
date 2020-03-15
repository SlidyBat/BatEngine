#include "PCH.h"
#include "RenderGraph.h"

#include "IRenderPass.h"

namespace Bat
{
	void RenderGraph::AddPass( const std::string& name, std::unique_ptr<IRenderPass> pass )
	{
		m_mapPassNameToIndex[name] = m_vRenderPasses.size();
		m_vPassNames.push_back( name );
		m_vRenderPasses.emplace_back( std::move( pass ) );
		m_vNodeAndResourceBindings.emplace_back();
		m_vPassEnabled.emplace_back( true );
	}

	size_t RenderGraph::GetPassCount() const
	{
		return m_vRenderPasses.size();
	}

	std::string RenderGraph::GetPassNameByIndex( size_t idx )
	{
		return m_vPassNames[idx];
	}

	IRenderPass* RenderGraph::GetPassByIndex( size_t idx )
	{
		ASSERT( idx < m_vRenderPasses.size(), "Invalid pass index" );
		return m_vRenderPasses[idx].get();
	}

	IRenderPass* RenderGraph::GetPassByName( const std::string& name )
	{
		int idx = GetPassIndexByName( name );
		if( idx == -1 )
		{
			return nullptr;
		}

		return GetPassByIndex( idx );
	}

	int RenderGraph::GetPassIndexByName( const std::string& name )
	{
		auto it = m_mapPassNameToIndex.find( name );
		if( it != m_mapPassNameToIndex.end() )
		{
			return (int)it->second;
		}
		return -1;
	}

	bool RenderGraph::IsPassEnabled( const std::string& name )
	{
		return IsPassEnabled( GetPassIndexByName( name ) );
	}

	bool RenderGraph::IsPassEnabled( size_t idx )
	{
		return m_vPassEnabled[idx];
	}

	void RenderGraph::SetPassEnabled( const std::string& name, bool enabled )
	{
		SetPassEnabled( GetPassIndexByName( name ), enabled );
	}

	void RenderGraph::SetPassEnabled( size_t idx, bool enabled )
	{
		m_vPassEnabled[idx] = enabled;
	}

#define RENDER_NODE_DATATYPE( type, name, capname ) \
	void RenderGraph::Add##name##Resource( const std::string& resource_name, std::unique_ptr<type> pResource ) \
	{ \
		ASSERT( GetResourceType( resource_name ) == NodeDataType::INVALID, "Resource added twice" ); \
		m_mapOwning##name[resource_name] = std::move( pResource ); \
		m_mapResourceTypes[resource_name] = NodeDataType::capname; \
	} \
	void RenderGraph::Add##name##Resource( const std::string& resource_name, type* pResource ) \
	{ \
		ASSERT( GetResourceType( resource_name ) == NodeDataType::INVALID, "Resource added twice" ); \
		m_mapNonOwning##name[resource_name] = pResource; \
		m_mapResourceTypes[resource_name] = NodeDataType::capname; \
	}
#include "RenderNodeDataTypes.def"

	void RenderGraph::BindToResource( const std::string& node_name, const std::string& resource )
	{
		std::optional<Node_t> node = CreateNodeFromString( node_name );
		if( !node )
		{
			BAT_ERROR( "Failed to create node '%s'", node_name );
			return;
		}

		NodeDataType resource_type = GetResourceType( resource );
		if( resource_type == NodeDataType::INVALID )
		{
			BAT_ERROR( "Failed to bind node '%s' to non-existent resource '%s'", node_name, resource );
			ASSERT( false, "" );
			return;
		}

		IRenderPass* pass = GetPassByIndex( node->pass_idx );
		if( pass->GetNodeDataType( node->name ) != resource_type )
		{
			BAT_ERROR( "Failed to bind node '%s' to resource '%s'. Mismatched types.", node_name, resource );
			ASSERT( false, "Failed to bind node '%s' to resource '%s'. Mismatched types.", node_name, resource );
			return;
		}

		NodeAndResource nar;
		nar.node = *node;
		nar.resource = resource;
		m_vNodeAndResourceBindings[node->pass_idx].emplace_back( nar );
	}

	void RenderGraph::MarkOutput( const std::string& out )
	{
		m_OutputNode = CreateNodeFromString( out, NodeType::OUTPUT );
		if( !m_OutputNode )
		{
			BAT_ERROR( "Failed to bind node '%s' to output", out );
			ASSERT( false, "" );
			return;
		}
	}

	void RenderGraph::Render( Camera& camera, SceneNode& scene, IRenderTarget* target )
	{
		if( !m_OutputNode )
		{
			ASSERT( false, "Render graph has no output node" );
			return;
		}

		for( size_t i = 0; i < m_vRenderPasses.size(); i++ )
		{
			if( !IsPassEnabled( i ) )
			{
				continue;
			}

			m_RenderData.Clear();

			// bind to output if this is the output pass
			if( i == (size_t)m_OutputNode->pass_idx )
			{
				m_RenderData.AddRenderTarget( m_OutputNode->name, target );
			}

			// bind any resources
			const auto& nodes_and_resources = m_vNodeAndResourceBindings[i];
			for( const NodeAndResource& nar : nodes_and_resources )
			{
				NodeDataType type = GetResourceType( nar.resource );
				switch( type )
				{
					case NodeDataType::TEXTURE:
						m_RenderData.AddTexture( nar.node.name, GetTextureResource( nar.resource ) );
						break;
					case NodeDataType::RENDER_TARGET:
						m_RenderData.AddRenderTarget( nar.node.name, GetRenderTargetResource( nar.resource ) );
						break;
					case NodeDataType::DEPTH_STENCIL:
						m_RenderData.AddDepthStencil( nar.node.name, GetDepthStencilResource( nar.resource ) );
						break;
					default:
						ASSERT( false, "Unhandled node data type" );
						break;
				}
			}

			IGPUContext* pContext = gpu->GetContext();

			pContext->BeginEvent( m_vPassNames[i] );
			m_vRenderPasses[i]->Execute( pContext, camera, scene, m_RenderData );
			pContext->EndEvent();
		}
	}

	void RenderGraph::ResetResources()
	{
#define RENDER_NODE_DATATYPE( type, name, capname ) \
		m_mapOwning##name.clear(); \
		m_mapNonOwning##name.clear();
#include "RenderNodeDataTypes.def"

		m_mapResourceTypes.clear();
		m_vNodeAndResourceBindings.clear();
	}

	void RenderGraph::ResetPasses()
	{
		m_vRenderPasses.clear();
		m_vPassNames.clear();
		m_mapPassNameToIndex.clear();
		m_vPassEnabled.clear();
		m_OutputNode.reset();
	}

	void RenderGraph::Reset()
	{
		ResetResources();
		ResetPasses();
	}

	std::optional<RenderGraph::Node_t> RenderGraph::CreateNodeFromString( const std::string& str, NodeType expected_type )
	{
		std::vector<std::string> split = Bat::SplitString( str, '.' );
		if( split.size() != 2 )
		{
			ASSERT( false, "Invalid node string '%s'", str );
			return {};
		}

		int pass_idx = GetPassIndexByName( split[0] );
		if( pass_idx == -1 )
		{
			ASSERT( false, "Invalid pass name '%s'", split[0] );
			return {};
		}

		IRenderPass* pass = GetPassByIndex( (size_t)pass_idx );

		NodeType type = pass->GetNodeType( split[1] );
		if( type == NodeType::INVALID )
		{
			ASSERT( false, "Invalid node '%s' (does not exist in specified pass)", split[1] );
			return {};
		}

		if( expected_type != NodeType::INVALID && type != expected_type )
		{
			ASSERT( false, "Invalid node '%s' (differs from expected node type)", split[1] );
			return {};
		}

		Node_t node;
		node.pass_idx = pass_idx;
		node.name = split[1];

		return node;
	}

	NodeDataType RenderGraph::GetResourceType( const std::string& resource_name )
	{
		auto it = m_mapResourceTypes.find( resource_name );
		if( it != m_mapResourceTypes.end() )
		{
			return it->second;
		}

		return NodeDataType::INVALID;
	}

#define RENDER_NODE_DATATYPE( type, name, capname ) \
	type* RenderGraph::Get##name##Resource( const std::string& resource_name ) \
	{ \
		auto it = m_mapOwning##name.find( resource_name ); \
		if( it != m_mapOwning##name.end() ) \
		{ \
			return it->second.get(); \
		} \
		auto it2 = m_mapNonOwning##name.find( resource_name ); \
		if( it2 != m_mapNonOwning##name.end() ) \
		{ \
			return it2->second; \
		} \
		return nullptr; \
	}
#include "RenderNodeDataTypes.def"
}
