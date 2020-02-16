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
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TARGET );
		}

		bool IsDynamicSkyEnabled() const { return m_bDynamicSky; }
		void SetDynamicSkyEnabled( bool enabled ) { m_bDynamicSky = enabled; }

		virtual std::string GetDescription() const override
		{
			return "Renders skybox. Ideally this should be after all geometry has been rendered so that most of the pixels are occluded";
		}

		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data ) override
		{
			pContext->SetDepthEnabled( true );
			pContext->SetBlendingEnabled( false );
			pContext->SetCullMode( CullMode::BACK );

			IRenderTarget* target = data.GetRenderTarget( "dst" );
			pContext->SetRenderTarget( target );

			if( m_bDynamicSky )
			{
				IVertexShader* pVertexShader = ResourceManager::GetVertexShader( "Graphics/Shaders/DynamicSkyVS.hlsl" );
				IPixelShader* pPixelShader = ResourceManager::GetPixelShader( "Graphics/Shaders/DynamicSkyPS.hlsl" );

				pContext->SetVertexShader( pVertexShader );
				pContext->SetPixelShader( pPixelShader );

				pContext->Draw( 3 );
			}
			else
			{
				if( ITexture* pSkybox = data.GetTexture( "skyboxtex" ) )
				{
					auto pPipeline = ShaderManager::GetPipeline<SkyboxPipeline>();
					pPipeline->Render( pContext, camera, pSkybox );
				}
			}
		}
	private:
		bool m_bDynamicSky = false;
	};
}
