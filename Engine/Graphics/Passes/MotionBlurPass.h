#pragma once

#include "ResourceManager.h"
#include "IRenderPass.h"
#include "RenderData.h"
#include "TexturePipeline.h"
#include "Entity.h"
#include "Globals.h"

namespace Bat
{
	class MotionBlurPass : public BaseRenderPass
	{
	private:
		struct CB_Globals
		{
			DirectX::XMMATRIX inv_viewproj = DirectX::XMMatrixIdentity();
			DirectX::XMMATRIX prev_viewproj = DirectX::XMMatrixIdentity();
		};
	public:
		MotionBlurPass()
		{
			// initialize render nodes
			AddRenderNode( "src", NodeType::INPUT, NodeDataType::RENDER_TARGET );  // the initial texture that bloom should be applied to
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TARGET ); // the output render texture
			AddRenderNode( "depth", NodeType::INPUT, NodeDataType::DEPTH_STENCIL );       // depth buffer

			// initialize shaders
			m_pTextureVS = ResourceManager::GetVertexShader( "Graphics/Shaders/TextureVS.hlsl" );
			m_pMotionBlurPS = ResourceManager::GetPixelShader( "Graphics/Shaders/MotionBlurPS.hlsl" );

			// initialize buffers
			const float left   = -Graphics::InitialScreenWidth / 2.0f;
			const float right  =  Graphics::InitialScreenWidth / 2.0f;
			const float top    =  Graphics::InitialScreenHeight / 2.0f;
			const float bottom = -Graphics::InitialScreenHeight / 2.0f;

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

		virtual std::string GetDescription() const override { return "Motion blur pass"; }

		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data )
		{
			pContext->SetDepthStencilEnabled( false );
			pContext->SetBlendingEnabled( false );

			pContext->SetPrimitiveTopology( PrimitiveTopology::TRIANGLELIST );

			IRenderTarget* src = data.GetRenderTarget( "src" );
			IRenderTarget* dst = data.GetRenderTarget( "dst" );
			IDepthStencil* depth = data.GetDepthStencil( "depth" );

			size_t width = src->GetWidth();
			size_t height = src->GetHeight();

			CB_TexturePipelineMatrix transform;
			transform.viewproj = DirectX::XMMatrixOrthographicLH( (float)width, (float)height, Graphics::ScreenNear, Graphics::ScreenFar );
			transform.world = DirectX::XMMatrixIdentity();
			m_cbufTransform.Update( pContext, transform );
			pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, 0 );

			CB_Globals ps_globals;
			DirectX::XMMATRIX viewproj = camera.GetViewMatrix() * camera.GetProjectionMatrix();
			ps_globals.inv_viewproj = DirectX::XMMatrixInverse( nullptr, viewproj );
			ps_globals.prev_viewproj = m_matPrevViewProj;
			m_cbufGlobals.Update( pContext, ps_globals );
			pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufGlobals, PS_CBUF_SLOT_0 );

			pContext->SetVertexBuffer( m_bufPosition, 0 );
			pContext->SetVertexBuffer( m_bufUV, 1 );
			pContext->SetIndexBuffer( m_bufIndices );

			pContext->SetVertexShader( m_pTextureVS.get() );
			pContext->SetPixelShader( m_pMotionBlurPS.get() );

			pContext->SetRenderTarget( dst );
			pContext->BindTexture( src, 0 );
			pContext->SetDepthStencil( nullptr ); // Unbind depth stencil from output
			pContext->BindTexture( depth, 1 ); // Bind to input

			pContext->DrawIndexed( m_bufIndices->GetIndexCount() );

			// Unbind depth stencil from input again
			pContext->UnbindTextureSlot( 1 );

			m_matPrevViewProj = viewproj;
		}
	private:
		DirectX::XMMATRIX m_matPrevViewProj = DirectX::XMMatrixIdentity();

		Resource<IVertexShader> m_pTextureVS;
		Resource<IPixelShader> m_pMotionBlurPS;

		VertexBuffer<Vec4> m_bufPosition;
		VertexBuffer<Vec2> m_bufUV;
		IndexBuffer m_bufIndices;

		ConstantBuffer<CB_TexturePipelineMatrix> m_cbufTransform;
		ConstantBuffer<CB_Globals> m_cbufGlobals;
	};
}