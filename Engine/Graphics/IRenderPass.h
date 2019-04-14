#pragma once

#include <string>
#include <vector>

#include "RenderNode.h"

namespace Bat
{
	class RenderData;
	class SceneGraph;
	class IGPUContext;

	class IRenderPass
	{
	public:
		virtual ~IRenderPass() = default;

		virtual std::string GetDescription() const = 0;
		virtual const std::vector<RenderNode>& GetNodes() const = 0;
		virtual NodeType GetNodeType( const std::string& name ) const = 0;
		virtual NodeDataType GetNodeDataType( const std::string& name ) const = 0;
		virtual void Execute( IGPUContext* pContext, SceneGraph& scene, RenderData& data ) = 0;
	};

	class BaseRenderPass : public IRenderPass
	{
	public:
		virtual std::string GetDescription() const override { return "N/A"; }
		virtual const std::vector<RenderNode>& GetNodes() const override;
		virtual NodeType GetNodeType( const std::string& name ) const override;
		virtual NodeDataType GetNodeDataType( const std::string& name ) const override;
	protected:
		void AddRenderNode( std::string name, NodeType type, NodeDataType datatype );
	private:
		std::vector<RenderNode> m_vNodes;
	};
}