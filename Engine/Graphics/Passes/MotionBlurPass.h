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
			Mat4 inv_viewproj = Mat4::Identity();
			Mat4 prev_viewproj = Mat4::Identity();
		};
	public:
		MotionBlurPass()
		{
			// initialize render nodes
			AddRenderNode( "src", NodeType::INPUT, NodeDataType::RENDER_TARGET );  // the initial texture that bloom should be applied to
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TARGET ); // the output render texture
			AddRenderNode( "depth", NodeType::INPUT, NodeDataType::DEPTH_STENCIL );       // depth buffer
		}

		virtual std::string GetDescription() const override { return "Motion blur pass"; }

		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data )
		{
			IVertexShader* pTextureVS = ResourceManager::GetVertexShader( "Graphics/Shaders/PostProcessVS.hlsl" );
			IPixelShader* pMotionBlurPS = ResourceManager::GetPixelShader( "Graphics/Shaders/MotionBlurPS.hlsl" );

			pContext->SetDepthEnabled( false );
			pContext->SetBlendingEnabled( false );

			pContext->SetPrimitiveTopology( PrimitiveTopology::TRIANGLELIST );

			IRenderTarget* src = data.GetRenderTarget( "src" );
			IRenderTarget* dst = data.GetRenderTarget( "dst" );
			IDepthStencil* depth = data.GetDepthStencil( "depth" );

			size_t width = src->GetWidth();
			size_t height = src->GetHeight();

			CB_Globals ps_globals;
			Mat4 viewproj = camera.GetViewMatrix() * camera.GetProjectionMatrix();
			ps_globals.inv_viewproj = Mat4::Inverse( viewproj );
			ps_globals.prev_viewproj = m_matPrevViewProj;
			m_cbufGlobals.Update( pContext, ps_globals );
			pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufGlobals, PS_CBUF_SLOT_0 );

			pContext->SetVertexShader( pTextureVS );
			pContext->SetPixelShader( pMotionBlurPS );

			pContext->SetRenderTarget( dst );
			pContext->BindTexture( src, PS_TEX_SLOT_0 );
			pContext->SetDepthStencil( nullptr ); // Unbind depth stencil from output
			pContext->BindTexture( depth, PS_TEX_SLOT_1 ); // Bind to input

			pContext->Draw( 3 );

			// Unbind depth stencil from input again
			pContext->UnbindTextureSlot( PS_TEX_SLOT_1 );

			m_matPrevViewProj = viewproj;
		}
	private:
		Mat4 m_matPrevViewProj = Mat4::Identity();

		ConstantBuffer<CB_Globals> m_cbufGlobals;
	};
}