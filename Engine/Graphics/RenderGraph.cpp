#include "PCH.h"
#include "RenderGraph.h"

#include "IRenderPass.h"
#include "RenderData.h"

namespace Bat
{
	void RenderGraph::AddPass( const std::string& name, std::unique_ptr<IRenderPass> pass )
	{
		m_mapPassNameToIndex[name] = m_vRenderPasses.size();
		m_vRenderPasses.emplace_back( std::move( pass ) );
		m_vNodeAndResourceBindings.emplace_back();
		m_vPassEnabled.emplace_back( true );
	}

	size_t RenderGraph::GetPassCount() const
	{
		return m_vRenderPasses.size();
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

	void RenderGraph::AddTextureResource( const std::string& name, std::unique_ptr<ITexture> pTexture )
	{
		m_mapTextures[name] = std::move( pTexture );
		m_mapResourceTypes[name] = NodeDataType::TEXTURE;
	}

	void RenderGraph::AddRenderTextureResource( const std::string& name, std::unique_ptr<IRenderTarget> pRenderTexture )
	{
		m_mapRenderTextures[name] = std::move( pRenderTexture );
		m_mapResourceTypes[name] = NodeDataType::RENDER_TEXTURE;
	}

	void RenderGraph::AddDepthStencilResource( const std::string& name, std::unique_ptr<IDepthStencil> pDepthStencil )
	{
		m_mapDepthStencils[name] = std::move( pDepthStencil );
		m_mapResourceTypes[name] = NodeDataType::DEPTH_STENCIL;
	}

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

			RenderData data;

			// bind to output if this is the output pass
			if( i == (size_t)m_OutputNode->pass_idx )
			{
				data.AddRenderTarget( m_OutputNode->name, target );
			}

			// bind any resources
			const auto& nodes_and_resources = m_vNodeAndResourceBindings[i];
			for( const NodeAndResource& nar : nodes_and_resources )
			{
				NodeDataType type = GetResourceType( nar.resource );
				switch( type )
				{
					case NodeDataType::TEXTURE:
						data.AddTexture( nar.node.name, GetTextureResource( nar.resource ) );
						break;
					case NodeDataType::RENDER_TEXTURE:
						data.AddRenderTarget( nar.node.name, GetRenderTextureResource( nar.resource ) );
						break;
					case NodeDataType::DEPTH_STENCIL:
						data.AddDepthStencil( nar.node.name, GetDepthStencilResource( nar.resource ) );
						break;
					default:
						ASSERT( false, "Unhandled node data type" );
						break;
				}
			}

			// run the pass
			m_vRenderPasses[i]->Execute( gpu->GetContext(), camera, scene, data );
		}
	}

	void RenderGraph::ResetResources()
	{
		m_mapTextures.clear();
		m_mapRenderTextures.clear();
		m_mapResourceTypes.clear();
		m_vNodeAndResourceBindings.clear();
	}

	void RenderGraph::ResetPasses()
	{
		m_vRenderPasses.clear();
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

	NodeDataType RenderGraph::GetResourceType( const std::string& name )
	{
		auto it = m_mapResourceTypes.find( name );
		if( it != m_mapResourceTypes.end() )
		{
			return it->second;
		}

		return NodeDataType::INVALID;
	}

	ITexture* RenderGraph::GetTextureResource( const std::string& name )
	{
		auto it = m_mapTextures.find( name );
		if( it != m_mapTextures.end() )
		{
			return it->second.get();
		}

		return nullptr;
	}

	IRenderTarget* RenderGraph::GetRenderTextureResource( const std::string& name )
	{
		auto it = m_mapRenderTextures.find( name );
		if( it != m_mapRenderTextures.end() )
		{
			return it->second.get();
		}

		return nullptr;
	}

	IDepthStencil* RenderGraph::GetDepthStencilResource( const std::string& name )
	{
		auto it = m_mapDepthStencils.find( name );
		if( it != m_mapDepthStencils.end() )
		{
			return it->second.get();
		}

		return nullptr;
	}
}
