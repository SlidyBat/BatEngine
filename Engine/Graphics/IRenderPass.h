#pragma once

#include <string>
#include <vector>
#include <stack>

#include "Core/Entity.h"
#include "RenderNode.h"

namespace Bat
{
	class RenderData;
	class IGPUContext;
	class Camera;
	class SceneNode;

	class IRenderPass
	{
	public:
		virtual ~IRenderPass() = default;

		virtual std::string GetDescription() const = 0;
		virtual const std::vector<RenderNode>& GetNodes() const = 0;
		virtual NodeType GetNodeType( const std::string& name ) const = 0;
		virtual NodeDataType GetNodeDataType( const std::string& name ) const = 0;
		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data ) = 0;
	};

	class BaseRenderPass : public IRenderPass
	{
	public:
		virtual std::string GetDescription() const override { return "N/A"; }
		virtual const std::vector<RenderNode>& GetNodes() const override;
		virtual NodeType GetNodeType( const std::string& name ) const override;
		virtual NodeDataType GetNodeDataType( const std::string& name ) const override;

		virtual void Visit( const Mat3x4& transform, Entity e ) {};
	protected:
		void AddRenderNode( std::string name, NodeType type, NodeDataType datatype );

		void Traverse();
	private:
		std::vector<RenderNode> m_vNodes;

		std::vector<const SceneNode*> m_NodeStack;
		std::vector<Mat4> m_Transforms;
	};
}