#pragma once

#include "IRenderPass.h"
#include "RenderTexture.h"
#include "RenderData.h"

namespace Bat
{
	class ClearRenderTargetPass : public BaseRenderPass
	{
	public:
		ClearRenderTargetPass()
		{
			AddRenderNode( "buffer", NodeType::INPUT, NodeDataType::RENDER_TEXTURE );
		}

		virtual std::string GetDescription() const override { return "Clears render target"; }

		virtual void Execute( SceneGraph& scene, RenderData& data ) override
		{
			RenderTexture* target = data.GetRenderTexture( "buffer" );
			target->Clear( 0.0f, 0.0f, 0.0f, 1.0f );
		}
	};
}
