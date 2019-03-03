#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

#include "RenderNode.h"

namespace Bat
{
	class IRenderPass;
	class SceneGraph;
	class Texture;
	class RenderTexture;

	class RenderGraph
	{
	public:
		void AddPass( const std::string& name, std::unique_ptr<IRenderPass> pass );
		size_t GetPassCount() const;
		IRenderPass* GetPassByIndex( size_t idx );
		IRenderPass* GetPassByName( const std::string& name );
		int GetPassIndexByName( const std::string& name );
		bool IsPassEnabled( const std::string& name );
		bool IsPassEnabled( size_t idx );
		void SetPassEnabled( const std::string& name, bool enabled );
		void SetPassEnabled( size_t idx, bool enabled );

		// Takes ownership of a texture and binds it to a certain name so it may be used by passes
		void AddTextureResource( const std::string& name, std::unique_ptr<Texture> pTexture );
		// Takes ownership of a render texture and binds it to a certain name so it may be used by passes
		void AddRenderTextureResource( const std::string& name, std::unique_ptr<RenderTexture> pTexture );
		// Binds a pass node to a created resource
		// Example usage:
		//     BindToResource( "depthprepass.dst", "DepthPrePassBuffer" );
		void BindToResource( const std::string& node_name, const std::string& resource );
		// Binds an output pass node to the final render target of the render graph (usually the backbuffer)
		// Example usage:
		//     MarkOutput( "Bloom.src" );
		void MarkOutput( const std::string& out );

		void Render( SceneGraph& scene, RenderTexture& target );

		// Clears all current resources
		void ResetResources();
		// Clears all current passes
		void ResetPasses();
		// Clears both resources & passes
		void Reset();
	private:
		struct Node_t
		{
			int pass_idx;
			std::string name;
		};

		struct NodeAndResource
		{
			Node_t node;
			std::string resource;
		};

		std::optional<Node_t> CreateNodeFromString( const std::string& str, NodeType expected_type = NodeType::INVALID );

		NodeDataType GetResourceType( const std::string& name );
		Texture* GetTextureResource( const std::string& name );
		RenderTexture* GetRenderTextureResource( const std::string& name );
	private:
		// passes
		std::vector<std::unique_ptr<IRenderPass>> m_vRenderPasses;
		std::unordered_map<std::string, size_t> m_mapPassNameToIndex;
		std::vector<bool> m_vPassEnabled;
		std::optional<Node_t> m_OutputNode;

		// resources
		std::unordered_map<std::string, std::unique_ptr<Texture>> m_mapTextures;
		std::unordered_map<std::string, std::unique_ptr<RenderTexture>> m_mapRenderTextures;
		std::unordered_map<std::string, NodeDataType> m_mapResourceTypes;
		std::vector<std::vector<NodeAndResource>> m_vNodeAndResourceBindings;
	};
}