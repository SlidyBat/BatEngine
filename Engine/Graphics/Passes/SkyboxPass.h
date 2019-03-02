#pragma once

#include "IRenderPass.h"
#include "RenderTexture.h"
#include "RenderData.h"
#include "SkyboxPipeline.h"
#include "MathLib.h"
#include "Camera.h"
#include "Scene.h"
#include "ShaderManager.h"
#include "RenderContext.h"

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

		virtual void Execute( SceneGraph& scene, RenderData& data ) override
		{
			RenderContext::SetDepthStencilEnabled( true );

			RenderTexture* target = data.GetRenderTexture( "dst" );
			target->Bind();

			Texture* pSkybox = data.GetTexture( "skyboxtex" );
			if( pSkybox )
			{
				Camera* cam = scene.GetActiveCamera();
				auto pos = cam->GetPosition();
				auto w = DirectX::XMMatrixTranslation( pos.x, pos.y, pos.z );
				auto t = w * cam->GetViewMatrix() * cam->GetProjectionMatrix();

				SkyboxPipelineParameters params( t, pSkybox->GetTextureView() );
				auto pPipeline = ShaderManager::GetPipeline( "skybox" );
				pPipeline->BindParameters( params );
				pPipeline->RenderIndexed( 0 ); // skybox uses its own index buffer & index count, doesnt matter what we pass in
			}
		}
	};
}
