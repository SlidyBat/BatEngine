#pragma once

#include "IRenderPass.h"
#include "RenderData.h"

namespace Bat
{
	class MsaaResolvePass : public BaseRenderPass
	{
	public:
		MsaaResolvePass()
		{
			AddRenderNode( "src", NodeType::INPUT, NodeDataType::RENDER_TARGET );
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TARGET );
		}

		virtual std::string GetDescription() const override { return "Resolves a multisampled render target into a non-multisampled render target"; }

		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data ) override
		{
			IRenderTarget* src = data.GetRenderTarget( "src" );
			IRenderTarget* dst = data.GetRenderTarget( "dst" );
			pContext->Resolve( dst, src );
		}
	};
}
