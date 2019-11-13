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
			float A = 0.15;
			float B = 0.50;
			float C = 0.10;
			float D = 0.20;
			float E = 0.02;
			float F = 0.30;
			float W = 11.2;
		};
	public:
		ToneMappingPass()
		{
			// initialize render nodes
			AddRenderNode( "src", NodeType::INPUT, NodeDataType::RENDER_TARGET );     // the initial texture that bloom should be applied to
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TARGET );    // the output render texture (can re-use buffer2 if needed)

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

		virtual std::string GetDescription() const override { return "Tone mapping & gamma correction"; }

		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data )
		{
			if( !m_bInitialized )
			{
				m_bInitialized = true;
				m_cbufToneMapSettings.Update( pContext, {} );
			}

			IVertexShader* pTextureVS = ResourceManager::GetVertexShader( "Graphics/Shaders/TextureVS.hlsl" );
			IPixelShader* pToneMapShader = ResourceManager::GetPixelShader( "Graphics/Shaders/ToneMappingPS.hlsl" );

			pContext->SetPrimitiveTopology( PrimitiveTopology::TRIANGLELIST );
			pContext->SetDepthStencilEnabled( false );
			pContext->SetDepthStencil( nullptr );
			pContext->SetBlendingEnabled( false );

			IRenderTarget* src = data.GetRenderTarget( "src" );
			IRenderTarget* dst = data.GetRenderTarget( "dst" );

			CB_TexturePipelineMatrix transform;
			transform.viewproj = DirectX::XMMatrixIdentity();
			transform.world = DirectX::XMMatrixIdentity();
			m_cbufTransform.Update( pContext, transform );
			pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, VS_CBUF_TRANSFORMS );

			pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufToneMapSettings, PS_CBUF_SLOT_0 );

			pContext->SetVertexBuffer( m_bufPosition, 0 );
			pContext->SetVertexBuffer( m_bufUV, 1 );
			pContext->SetIndexBuffer( m_bufIndices );

			pContext->SetVertexShader( pTextureVS );

			pContext->SetRenderTarget( dst );
			pContext->SetPixelShader( pToneMapShader );
			pContext->BindTexture( src, 0 );
			pContext->DrawIndexed( m_bufIndices->GetIndexCount() );
		}
	private:
		bool m_bInitialized = false;
		ConstantBuffer<CB_TexturePipelineMatrix> m_cbufTransform;
		ConstantBuffer<CB_ToneMapSettings> m_cbufToneMapSettings;

		VertexBuffer<Vec4> m_bufPosition;
		VertexBuffer<Vec2> m_bufUV;
		IndexBuffer m_bufIndices;
	};
}