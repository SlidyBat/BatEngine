#pragma once

#include "ResourceManager.h"
#include "IRenderPass.h"
#include "RenderData.h"
#include "RenderContext.h"
#include "TexturePipeline.h"
#include "Globals.h"

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
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TEXTURE );    // the output render texture (can re-use buffer1 if needed)

			// initialize shaders
			m_pTextureVS = ResourceManager::GetVertexShader( "Graphics/Shaders/TextureVS.hlsl" );
			m_pTextureVS->AddConstantBuffer<CB_TexturePipelineMatrix>();
			m_pTexturePS = ResourceManager::GetPixelShader( "Graphics/Shaders/TexturePS.hlsl" );
			m_pBrightExtractPS = ResourceManager::GetPixelShader( "Graphics/Shaders/BrightExtractPS.hlsl" );
			m_pBrightExtractPS->AddConstantBuffer<CB_Globals>();
			m_pGaussBlurHorPS = ResourceManager::GetPixelShader( "Graphics/Shaders/GaussBlurHorPS.hlsl" );
			m_pGaussBlurHorPS->AddConstantBuffer<CB_Globals>();
			m_pGaussBlurVerPS = ResourceManager::GetPixelShader( "Graphics/Shaders/GaussBlurVerPS.hlsl" );
			m_pGaussBlurVerPS->AddConstantBuffer<CB_Globals>();

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

			const std::vector<int> indices = { 0, 1, 2,  2, 3, 0 };

			m_bufPosition.SetData( positions );
			m_bufUV.SetData( uvs );
			m_bufIndices.SetData( indices );
		}

		virtual std::string GetDescription() const override { return "Bloom pass"; }

		virtual void Execute( SceneGraph& scene, RenderData& data )
		{
			RenderTexture* src = data.GetRenderTexture( "src" );
			RenderTexture* rt1 = data.GetRenderTexture( "buffer1" );
			RenderTexture* rt2 = data.GetRenderTexture( "buffer2" );
			RenderTexture* dst = data.GetRenderTexture( "dst" );

			int width = src->GetTextureWidth();
			int height = src->GetTextureHeight();

			CB_TexturePipelineMatrix transform;
			transform.viewproj = DirectX::XMMatrixOrthographicLH( (float)width, (float)height, 0.1f, 1000.0f );
			transform.world = DirectX::XMMatrixIdentity();
			RenderContext::GetDeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

			CB_Globals psGlobals;
			psGlobals.resolution = { (float)width, (float)height };
			psGlobals.time = g_pGlobals->elapsed_time;

			m_pBrightExtractPS->GetConstantBuffer( 0 ).SetData( &psGlobals );
			m_pGaussBlurHorPS->GetConstantBuffer( 0 ).SetData( &psGlobals );
			m_pGaussBlurVerPS->GetConstantBuffer( 0 ).SetData( &psGlobals );

			m_bufPosition.Bind( 0 );
			m_bufUV.Bind( 1 );
			m_bufIndices.Bind();

			m_pTextureVS->Bind();
			m_pTextureVS->GetConstantBuffer( 0 ).SetData( &transform );

			rt1->Bind();
			m_pBrightExtractPS->Bind();
			m_pBrightExtractPS->SetResource( 0, src->GetTextureView() );
			RenderContext::GetDeviceContext()->DrawIndexed( m_bufIndices.GetIndexCount(), 0, 0 );
			RenderContext::FlushMessages();

			for( int i = 0; i < m_iBlurPasses * 2; i++ )
			{
				RenderTexture::UnbindAll();
				if( i % 2 == 0 )
				{
					m_pGaussBlurHorPS->Bind();
					m_pGaussBlurHorPS->SetResource( 0, rt1->GetTextureView() );
					rt2->Bind();
				}
				else
				{
					m_pGaussBlurVerPS->Bind();
					m_pGaussBlurVerPS->SetResource( 0, rt2->GetTextureView() );
					rt1->Bind();
				}
				RenderContext::GetDeviceContext()->DrawIndexed( m_bufIndices.GetIndexCount(), 0, 0 );
				RenderContext::FlushMessages();
			}

			RenderTexture::UnbindAll();
			m_pBloomShader->Bind();
			m_pBloomShader->SetResource( 0, src->GetTextureView() );
			m_pBloomShader->SetResource( 1, rt1->GetTextureView() );
			dst->Bind();
			RenderContext::GetDeviceContext()->DrawIndexed( m_bufIndices.GetIndexCount(), 0, 0 );
			RenderContext::FlushMessages();
		}
	private:
		int m_iBlurPasses = 1;

		Resource<PixelShader> m_pTexturePS;
		Resource<VertexShader> m_pTextureVS;
		Resource<PixelShader> m_pBrightExtractPS;
		Resource<PixelShader> m_pGaussBlurHorPS;
		Resource<PixelShader> m_pGaussBlurVerPS;
		Resource<PixelShader> m_pBloomShader;

		VertexBuffer<Vec4> m_bufPosition;
		VertexBuffer<Vec2> m_bufUV;
		IndexBuffer m_bufIndices;
	};
}