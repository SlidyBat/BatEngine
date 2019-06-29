#pragma once

#include "ResourceManager.h"
#include "IRenderPass.h"
#include "RenderData.h"
#include "TexturePipeline.h"
#include "ConstantBuffer.h"

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
			AddRenderNode( "src", NodeType::INPUT, NodeDataType::RENDER_TEXTURE );     // the initial texture that bloom should be applied to
			AddRenderNode( "buffer1", NodeType::INPUT, NodeDataType::RENDER_TEXTURE ); // a frame buffer to use for the multiple blur passes
			AddRenderNode( "buffer2", NodeType::INPUT, NodeDataType::RENDER_TEXTURE ); // a frame buffer to use for the multiple blur passes
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TEXTURE );    // the output render texture (can re-use buffer2 if needed)

			// initialize shaders
			m_pTextureVS = ResourceManager::GetVertexShader( "Graphics/Shaders/TextureVS.hlsl" );
			m_pBrightExtractPS = ResourceManager::GetPixelShader( "Graphics/Shaders/BrightExtractPS.hlsl" );
			m_pGaussBlurPS = ResourceManager::GetPixelShader( "Graphics/Shaders/GaussBlurPS.hlsl" );

			m_pBloomShader = ResourceManager::GetPixelShader( "Graphics/Shaders/BloomPS.hlsl" );

			// initialize buffers
			const std::vector<Vec4> positions = {
				{ -1.0f, -1.0f, 1.0f, 1.0f },
				{ -1.0f,  1.0f, 1.0f, 1.0f },
				{  1.0f,  1.0f, 1.0f, 1.0f },
				{  1.0f, -1.0f, 1.0f, 1.0f }
			};

			const std::vector<Vec2> uvs = {
				{ 0.0f, 1.0f },
				{ 0.0f, 0.0f },
				{ 1.0f, 0.0f },
				{ 1.0f, 1.0f }
			};

			const std::vector<uint32_t> indices = { 0, 1, 2,  2, 3, 0 };

			m_bufPosition.Reset( positions );
			m_bufUV.Reset( uvs );
			m_bufIndices.Reset( indices );
		}

		virtual std::string GetDescription() const override { return "Bloom pass"; }

		void SetThreshold( float threshold ) { m_flThreshold = threshold; }

		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data )
		{
			pContext->SetDepthStencilEnabled( false );
			pContext->SetDepthStencil( nullptr );

			IRenderTarget* src = data.GetRenderTarget( "src" );
			IRenderTarget* rt1 = data.GetRenderTarget( "buffer1" );
			IRenderTarget* rt2 = data.GetRenderTarget( "buffer2" );
			IRenderTarget* dst = data.GetRenderTarget( "dst" );

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

			CB_TexturePipelineMatrix transform;
			transform.viewproj = DirectX::XMMatrixIdentity();
			transform.world = DirectX::XMMatrixIdentity();
			m_cbufTransform.Update( pContext, transform );
			pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, 0 );

			CB_ThresholdBuf threshold;
			threshold.BrightThreshold = m_flThreshold;
			m_cbufThreshold.Update( pContext, threshold );
			pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufThreshold, PS_CBUF_SLOT_0 );

			pContext->SetVertexBuffer( m_bufPosition, 0 );
			pContext->SetVertexBuffer( m_bufUV, 1 );
			pContext->SetIndexBuffer( m_bufIndices );

			pContext->SetVertexShader( m_pTextureVS.get() );

			pContext->SetRenderTarget( rt1 );
			pContext->SetPixelShader( m_pBrightExtractPS.get() );
			pContext->BindTexture( src, 0 );
			pContext->DrawIndexed( m_bufIndices->GetIndexCount() );

			CB_BlurSettings blur_settings;
			pContext->SetPixelShader( m_pGaussBlurPS.get() );
			pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufBlurSettings, PS_CBUF_SLOT_0 );
			for( int i = 0; i < m_iBlurPasses * 2; i++ )
			{
				// Unbind whatever is currently bound, to avoid binding on input/output at same time
				pContext->SetRenderTarget( nullptr );
				pContext->UnbindTextureSlot( 0 );

				if( i % 2 == 0 )
				{
					blur_settings.Horizontal = true;
					m_cbufBlurSettings.Update( pContext, blur_settings );

					pContext->BindTexture( rt1, 0 );
					pContext->SetRenderTarget( rt2 );
				}
				else
				{
					blur_settings.Horizontal = false;
					m_cbufBlurSettings.Update( pContext, blur_settings );

					pContext->BindTexture( rt2, 0 );
					pContext->SetRenderTarget( rt1 );
				}

				pContext->DrawIndexed( m_bufIndices->GetIndexCount() );
			}

			// Unbind whatever is currently bound, to avoid binding on input/output at same time
			pContext->SetRenderTarget( nullptr );
			pContext->UnbindTextureSlot( 0 );

			pContext->PopViewport();

			pContext->SetPixelShader( m_pBloomShader.get() );
			pContext->BindTexture( src, 0 );
			pContext->BindTexture( rt1, 1 );
			pContext->SetRenderTarget( dst );
			pContext->DrawIndexed( m_bufIndices->GetIndexCount() );

			// Unbind again
			pContext->UnbindTextureSlot( 0 );
			pContext->UnbindTextureSlot( 1 );
		}
	private:
		int m_iBlurPasses = 5;
		float m_flThreshold = 1.0f;

		Resource<IVertexShader> m_pTextureVS;
		Resource<IPixelShader> m_pBrightExtractPS;
		Resource<IPixelShader> m_pGaussBlurPS;
		Resource<IPixelShader> m_pBloomShader;

		ConstantBuffer<CB_TexturePipelineMatrix> m_cbufTransform;
		ConstantBuffer<CB_BlurSettings> m_cbufBlurSettings;
		ConstantBuffer<CB_ThresholdBuf> m_cbufThreshold;

		VertexBuffer<Vec4> m_bufPosition;
		VertexBuffer<Vec2> m_bufUV;
		IndexBuffer m_bufIndices;
	};
}