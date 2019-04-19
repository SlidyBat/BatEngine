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
		struct CB_Globals
		{
			Vec2 resolution;
			float time;
			float pad;
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
			m_pGaussBlurHorPS = ResourceManager::GetPixelShader( "Graphics/Shaders/GaussBlurHorPS.hlsl" );
			m_pGaussBlurVerPS = ResourceManager::GetPixelShader( "Graphics/Shaders/GaussBlurVerPS.hlsl" );

			m_pBloomShader = ResourceManager::GetPixelShader( "Graphics/Shaders/BloomPS.hlsl" );

			// initialize buffers
			const float left   = -400.0f;
			const float right  =  400.0f;
			const float top    =  300.0f;
			const float bottom = -300.0f;

			const std::vector<Vec4> positions = {
				{ left,  bottom, 1.0f, 1.0f },
				{ left,  top,    1.0f, 1.0f },
				{ right, top,    1.0f, 1.0f },
				{ right, bottom, 1.0f, 1.0f }
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

		virtual void Execute( IGPUContext* pContext, SceneGraph& scene, RenderData& data )
		{
			pContext->SetDepthStencilEnabled( false );

			IRenderTarget* src = data.GetRenderTarget( "src" );
			IRenderTarget* rt1 = data.GetRenderTarget( "buffer1" );
			IRenderTarget* rt2 = data.GetRenderTarget( "buffer2" );
			IRenderTarget* dst = data.GetRenderTarget( "dst" );

			size_t width = src->GetWidth();
			size_t height = src->GetHeight();

			pContext->SetPrimitiveTopology( PrimitiveTopology::TRIANGLELIST );

			CB_TexturePipelineMatrix transform;
			transform.viewproj = DirectX::XMMatrixOrthographicLH( (float)width, (float)height, Graphics::ScreenNear, Graphics::ScreenFar );
			transform.world = DirectX::XMMatrixIdentity();
			m_cbufTransform.Update( pContext, transform );
			pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, 0 );

			CB_Globals globals;
			globals.resolution = { (float)width, (float)height };
			globals.time = g_pGlobals->elapsed_time;
			m_cbufGlobals.Update( pContext, globals );
			pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufGlobals, 0 );

			pContext->SetVertexBuffer( m_bufPosition, 0 );
			pContext->SetVertexBuffer( m_bufUV, 1 );
			pContext->SetIndexBuffer( m_bufIndices );

			pContext->SetVertexShader( m_pTextureVS.get() );

			pContext->SetRenderTarget( rt1 );
			pContext->SetPixelShader( m_pBrightExtractPS.get() );
			pContext->BindTexture( src, 0 );
			pContext->DrawIndexed( m_bufIndices->GetIndexCount() );

			for( int i = 0; i < m_iBlurPasses * 2; i++ )
			{
				// Unbind whatever is currently bound, to avoid binding on input/output at same time
				pContext->SetRenderTarget( nullptr );
				pContext->UnbindTextureSlot( 0 );

				if( i % 2 == 0 )
				{
					pContext->SetPixelShader( m_pGaussBlurHorPS.get() );
					pContext->BindTexture( rt1, 0 );
					pContext->SetRenderTarget( rt2 );
				}
				else
				{
					pContext->SetPixelShader( m_pGaussBlurVerPS.get() );
					pContext->BindTexture( rt2, 0 );
					pContext->SetRenderTarget( rt1 );
				}

				pContext->DrawIndexed( m_bufIndices->GetIndexCount() );
			}

			// Unbind whatever is currently bound, to avoid binding on input/output at same time
			pContext->SetRenderTarget( nullptr );
			pContext->UnbindTextureSlot( 0 );

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
		int m_iBlurPasses = 1;

		Resource<IVertexShader> m_pTextureVS;
		Resource<IPixelShader> m_pBrightExtractPS;
		Resource<IPixelShader> m_pGaussBlurHorPS;
		Resource<IPixelShader> m_pGaussBlurVerPS;
		Resource<IPixelShader> m_pBloomShader;

		ConstantBuffer<CB_Globals> m_cbufGlobals;
		ConstantBuffer<CB_TexturePipelineMatrix> m_cbufTransform;

		VertexBuffer<Vec4> m_bufPosition;
		VertexBuffer<Vec2> m_bufUV;
		IndexBuffer m_bufIndices;
	};
}