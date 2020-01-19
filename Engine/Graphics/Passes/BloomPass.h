#pragma once

#include "ResourceManager.h"
#include "IRenderPass.h"
#include "RenderData.h"
#include "TexturePipeline.h"
#include "ConstantBuffer.h"
#include "ScratchRenderTarget.h"

namespace Bat
{
	class BloomPass : public BaseRenderPass
	{
	private:
		struct CB_ThresholdBuf
		{
			float BrightThreshold;
			float _pad0[3];
		};

		struct CB_BlurSettings
		{
			int Horizontal;
			float _pad0[3];
		};
	public:
		BloomPass( int blurpasses = 5 )
		{
			m_iBlurPasses = blurpasses;

			// initialize render nodes
			AddRenderNode( "src", NodeType::INPUT, NodeDataType::RENDER_TARGET );     // the initial texture that bloom should be applied to
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TARGET );    // the output render texture (can re-use buffer2 if needed)
		}

		virtual std::string GetDescription() const override { return "Bloom pass"; }

		void SetThreshold( float threshold ) { m_flThreshold = threshold; }

		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data )
		{
			IVertexShader* pPostProcessVS = ResourceManager::GetVertexShader( "Graphics/Shaders/PostProcessVS.hlsl" );
			IPixelShader* pBrightExtractPS = ResourceManager::GetPixelShader( "Graphics/Shaders/BrightExtractPS.hlsl" );
			IPixelShader* pGaussBlurPS = ResourceManager::GetPixelShader( "Graphics/Shaders/GaussBlurPS.hlsl" );
			IPixelShader* pBloomShader = ResourceManager::GetPixelShader( "Graphics/Shaders/BloomPS.hlsl" );

			pContext->SetDepthEnabled( false );
			pContext->SetDepthStencil( nullptr );
			pContext->SetBlendingEnabled( false );

			IRenderTarget* src = data.GetRenderTarget( "src" );
			IRenderTarget* dst = data.GetRenderTarget( "dst" );
			auto rt1 = ScratchRenderTarget::Create( src->GetWidth() / 2, src->GetHeight() / 2, src->GetFormat() );
			auto rt2 = ScratchRenderTarget::Create( src->GetWidth() / 2, src->GetHeight() / 2, src->GetFormat() );

			size_t width = src->GetWidth();
			size_t height = src->GetHeight();

			Viewport vp;
			vp.top_left = { 0.0f, 0.0f };
			vp.width = (float)rt1->GetWidth();
			vp.height = (float)rt1->GetHeight();
			vp.min_depth = 0.0f;
			vp.max_depth = 1.0f;
			pContext->PushViewport( vp );

			pContext->SetPrimitiveTopology( PrimitiveTopology::TRIANGLELIST );

			CB_ThresholdBuf threshold;
			threshold.BrightThreshold = m_flThreshold;
			m_cbufThreshold.Update( pContext, threshold );
			pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufThreshold, PS_CBUF_SLOT_0 );

			pContext->SetVertexShader( pPostProcessVS );

			pContext->SetRenderTarget( rt1 );
			pContext->SetPixelShader( pBrightExtractPS );
			pContext->BindTexture( src, PS_TEX_SLOT_0 );
			pContext->Draw( 3 );

			CB_BlurSettings blur_settings;
			pContext->SetPixelShader( pGaussBlurPS );
			pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufBlurSettings, PS_CBUF_SLOT_0 );
			for( int i = 0; i < m_iBlurPasses * 2; i++ )
			{
				// Unbind whatever is currently bound, to avoid binding on input/output at same time
				pContext->SetRenderTarget( nullptr );
				pContext->UnbindTextureSlot( PS_TEX_SLOT_0 );

				if( i % 2 == 0 )
				{
					blur_settings.Horizontal = true;
					m_cbufBlurSettings.Update( pContext, blur_settings );

					pContext->BindTexture( rt1, PS_TEX_SLOT_0 );
					pContext->SetRenderTarget( rt2 );
				}
				else
				{
					blur_settings.Horizontal = false;
					m_cbufBlurSettings.Update( pContext, blur_settings );

					pContext->BindTexture( rt2, PS_TEX_SLOT_0 );
					pContext->SetRenderTarget( rt1 );
				}

				pContext->Draw( 3 );
			}

			// Unbind whatever is currently bound, to avoid binding on input/output at same time
			pContext->SetRenderTarget( nullptr );
			pContext->UnbindTextureSlot( PS_TEX_SLOT_0 );

			pContext->PopViewport();

			pContext->SetPixelShader( pBloomShader );
			pContext->BindTexture( src, PS_TEX_SLOT_0 );
			pContext->BindTexture( rt1, PS_TEX_SLOT_1 );
			pContext->SetRenderTarget( dst );
			pContext->Draw( 3 );

			// Unbind again
			pContext->UnbindTextureSlot( PS_TEX_SLOT_0 );
			pContext->UnbindTextureSlot( PS_TEX_SLOT_1 );
		}
	private:
		int m_iBlurPasses = 5;
		float m_flThreshold = 1.0f;

		ConstantBuffer<CB_BlurSettings> m_cbufBlurSettings;
		ConstantBuffer<CB_ThresholdBuf> m_cbufThreshold;
	};
}