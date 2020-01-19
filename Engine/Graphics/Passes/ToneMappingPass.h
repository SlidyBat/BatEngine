#pragma once

#include "ResourceManager.h"
#include "IRenderPass.h"
#include "RenderData.h"
#include "TexturePipeline.h"
#include "ConstantBuffer.h"

namespace Bat
{
	class ToneMappingPass : public BaseRenderPass
	{
	private:
		struct CB_ToneMapSettings
		{
			float Exposure = 2.0f;
			float _pad0[3];
		};
	public:
		ToneMappingPass()
		{
			// initialize render nodes
			AddRenderNode( "src", NodeType::INPUT, NodeDataType::RENDER_TARGET );     // the initial texture that bloom should be applied to
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TARGET );    // the output render texture (can re-use buffer2 if needed)
		}

		virtual std::string GetDescription() const override { return "Tone mapping & gamma correction"; }

		void SetExposure( float exposure )
		{
			m_Settings.Exposure = exposure;
		}

		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data )
		{
			m_cbufToneMapSettings.Update( pContext, m_Settings );

			IVertexShader* pTextureVS = ResourceManager::GetVertexShader( "Graphics/Shaders/PostProcessVS.hlsl" );
			IPixelShader* pToneMapShader = ResourceManager::GetPixelShader( "Graphics/Shaders/ToneMappingPS.hlsl" );

			pContext->SetPrimitiveTopology( PrimitiveTopology::TRIANGLELIST );
			pContext->SetDepthEnabled( false );
			pContext->SetDepthStencil( nullptr );
			pContext->SetBlendingEnabled( false );

			IRenderTarget* src = data.GetRenderTarget( "src" );
			IRenderTarget* dst = data.GetRenderTarget( "dst" );

			pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufToneMapSettings, PS_CBUF_SLOT_0 );

			pContext->SetVertexShader( pTextureVS );
			pContext->SetPixelShader( pToneMapShader );

			pContext->SetRenderTarget( dst );
			pContext->BindTexture( src, PS_TEX_SLOT_0 );
			pContext->Draw( 3 );
		}
	private:
		bool m_bInitialized = false;
		CB_ToneMapSettings m_Settings;
		ConstantBuffer<CB_ToneMapSettings> m_cbufToneMapSettings;
	};
}