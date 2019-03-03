#include "PCH.h"
#include "RenderGraph.h"

#include "IRenderPass.h"
#include "RenderData.h"
#include "Texture.h"
#include "RenderTexture.h"

namespace Bat
{
	void RenderGraph::AddPass( const std::string& name, std::unique_ptr<IRenderPass> pass )
	{
		m_mapPassNameToIndex[name] = m_vRenderPasses.size();
		BAT_LOG( "m_mapPassNameToIndex[%s] = %i", name, m_vRenderPasses.size() );
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
		size_t idx = GetPassIndexByName( name );
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

	void RenderGraph::AddTextureResource( const std::string& name, std::unique_ptr<Texture> pTexture )
	{
		m_mapTextures[name] = std::move( pTexture );
		m_mapResourceTypes[name] = NodeDataType::TEXTURE;
	}

	void RenderGraph::AddRenderTextureResource( const std::string& name, std::unique_ptr<RenderTexture> pRenderTexture )
	{
		m_mapRenderTextures[name] = std::move( pRenderTexture );
		m_mapResourceTypes[name] = NodeDataType::RENDER_TEXTURE;
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
			ASSERT( false, "" );
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

	void RenderGraph::Render( SceneGraph& scene, RenderTexture& target )
	{
		if( !m_OutputNode )
		{
			ASSERT( false, "Render graph has no output node" );
			return;
		}

		for( int i = 0; i < m_vRenderPasses.size(); i++ )
		{
			RenderData data;

			// bind to output if this is the output pass
			if( i == m_OutputNode->pass_idx )
			{
				data.AddRenderTexture( m_OutputNode->name, &target );
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
						data.AddRenderTexture( nar.node.name, GetRenderTextureResource( nar.resource ) );
						break;
					default:
						ASSERT( false, "Unhandled node data type" );
						break;
				}
			}

			// run the pass
			m_vRenderPasses[i]->Execute( scene, data );
		}
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
		BAT_LOG( "%s (%i)", split[0], pass_idx );
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
	Texture* RenderGraph::GetTextureResource( const std::string& name )
	{
		auto it = m_mapTextures.find( name );
		if( it != m_mapTextures.end() )
		{
			return it->second.get();
		}

		return nullptr;
	}
	RenderTexture * RenderGraph::GetRenderTextureResource( const std::string & name )
	{
		auto it = m_mapRenderTextures.find( name );
		if( it != m_mapRenderTextures.end() )
		{
			return it->second.get();
		}

		return nullptr;
	}
}
