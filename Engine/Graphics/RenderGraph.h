#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

#include "Entity.h"
#include "Camera.h"
#include "RenderNode.h"
#include "RenderData.h"

namespace Bat
{
	class IRenderPass;
	class ITexture;
	class IRenderTarget;
	class IDepthStencil;

	class RenderGraph
	{
	public:
		void AddPass( const std::string& name, std::unique_ptr<IRenderPass> pass );
		size_t GetPassCount() const;
		IRenderPass* GetPassByIndex( size_t idx );
		std::string GetPassNameByIndex( size_t idx );
		IRenderPass* GetPassByName( const std::string& name );
		int GetPassIndexByName( const std::string& name );
		bool IsPassEnabled( const std::string& name );
		bool IsPassEnabled( size_t idx );
		void SetPassEnabled( const std::string& name, bool enabled );
		void SetPassEnabled( size_t idx, bool enabled );

#define RENDER_NODE_DATATYPE( type, name, capname ) \
		/* Takes ownership of resource and binds it to a certain name so it may be used by passes */ \
		void Add##name##Resource( const std::string& resource_name, std::unique_ptr<type> pResource ); \
		/* Uses resource without owning it and binds it to a certain name so it may be used by passes */ \
		void Add##name##Resource( const std::string& resource_name, type* pResource );
#include "RenderNodeDataTypes.def"

		// Binds a pass node to a created resource
		// Example usage:
		//     BindToResource( "depthprepass.dst", "DepthPrePassBuffer" );
		void BindToResource( const std::string& node_name, const std::string& resource );
		// Binds an output pass node to the final render target of the render graph (usually the backbuffer)
		// Example usage:
		//     MarkOutput( "Bloom.src" );
		void MarkOutput( const std::string& out );

		void Render( Camera& camera, SceneNode& scene, IRenderTarget* pTarget );

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

		NodeDataType GetResourceType( const std::string& resource_name );

#define RENDER_NODE_DATATYPE( type, name, capname ) \
		type* Get##name##Resource( const std::string& resource_name );
#include "RenderNodeDataTypes.def"
	private:
		// passes
		std::vector<std::unique_ptr<IRenderPass>> m_vRenderPasses;
		std::vector<std::string> m_vPassNames;
		std::unordered_map<std::string, size_t> m_mapPassNameToIndex;
		std::vector<bool> m_vPassEnabled;
		std::optional<Node_t> m_OutputNode;

		std::unordered_map<std::string, NodeDataType> m_mapResourceTypes;
		std::vector<std::vector<NodeAndResource>> m_vNodeAndResourceBindings;

		RenderData m_RenderData;

#define RENDER_NODE_DATATYPE( type, name, capname ) \
			std::unordered_map<std::string, std::unique_ptr<type>> m_mapOwning##name; \
			std::unordered_map<std::string, type*> m_mapNonOwning##name;
#include "RenderNodeDataTypes.def"
	};
}