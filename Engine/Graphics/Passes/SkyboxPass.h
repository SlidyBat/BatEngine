#pragma once

#include "IRenderPass.h"
#include "RenderData.h"
#include "SkyboxPipeline.h"
#include "MathLib.h"
#include "Camera.h"
#include "Entity.h"
#include "ShaderManager.h"

namespace Bat
{
	class SkyboxPass : public BaseRenderPass
	{
	public:
		SkyboxPass()
		{
			AddRenderNode( "skyboxtex", NodeType::INPUT, NodeDataType::TEXTURE );
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TEXTURE );
		}

		virtual std::string GetDescription() const override
		{
			return "Renders skybox. Ideally this should be after all geometry has been rendered so that most of the pixels are occluded";
		}

		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data ) override
		{
			pContext->SetDepthStencilEnabled( true );

			IRenderTarget* target = data.GetRenderTarget( "dst" );
			pContext->SetRenderTarget( target );

			if( ITexture* pSkybox = data.GetTexture( "skyboxtex" ) )
			{
				auto pos = camera.GetPosition();
				auto w = DirectX::XMMatrixTranslation( pos.x, pos.y, pos.z );
				auto t = w * camera.GetViewMatrix() * camera.GetProjectionMatrix();

				SkyboxPipelineParameters params( t, pSkybox );
				auto pPipeline = ShaderManager::GetPipeline( "skybox" );
				pPipeline->BindParameters( pContext, params );
				pPipeline->RenderIndexed( pContext, 0 ); // skybox uses its own index buffer & index count, doesnt matter what we pass in
			}
		}
	};
}
