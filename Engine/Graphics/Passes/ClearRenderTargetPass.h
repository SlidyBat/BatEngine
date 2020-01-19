#pragma once

#include "IRenderPass.h"
#include "RenderData.h"

namespace Bat
{
	class ClearRenderTargetPass : public BaseRenderPass
	{
	public:
		ClearRenderTargetPass()
		{
			AddRenderNode( "buffer", NodeType::INPUT, NodeDataType::RENDER_TARGET );
			AddRenderNode( "depth", NodeType::INPUT, NodeDataType::DEPTH_STENCIL );
		}

		virtual std::string GetDescription() const override { return "Clears render target and depth"; }

		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data ) override
		{
			IRenderTarget* target = data.GetRenderTarget( "buffer" );
			pContext->ClearRenderTarget( target, 0.0f, 0.0f, 0.0f, 1.0f );

			IDepthStencil* depth = data.GetDepthStencil( "depth" );
			if( depth )
			{
				pContext->SetRenderTarget( nullptr );
				pContext->SetDepthStencil( depth );
				pContext->ClearDepthStencil( depth, CLEAR_FLAG_DEPTH | CLEAR_FLAG_STENCIL, 1.0f, 0 );
			}
		}
	};
}
