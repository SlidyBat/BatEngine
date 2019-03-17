#pragma once

#include "ResourceManager.h"
#include "IRenderPass.h"
#include "RenderData.h"
#include "RenderContext.h"
#include "TexturePipeline.h"
#include "Scene.h"
#include "Globals.h"

namespace Bat
{
	class MotionBlurPass : public BaseRenderPass
	{
	private:
		struct CB_Globals
		{
			Vec2 resolution;
			float time;
			float pad;
			DirectX::XMMATRIX inv_viewproj;
			DirectX::XMMATRIX prev_viewproj;
		};
	public:
		MotionBlurPass()
		{
			// initialize render nodes
			AddRenderNode( "src", NodeType::INPUT, NodeDataType::RENDER_TEXTURE );  // the initial texture that bloom should be applied to
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TEXTURE ); // the output render texture
			AddRenderNode( "depth", NodeType::INPUT, NodeDataType::TEXTURE );       // depth buffer

			// initialize shaders
			m_pTextureVS = ResourceManager::GetVertexShader( "Graphics/Shaders/TextureVS.hlsl" );
			m_pTextureVS->AddConstantBuffer<CB_TexturePipelineMatrix>();
			m_pMotionBlurPS = ResourceManager::GetPixelShader( "Graphics/Shaders/MotionBlurPS.hlsl" );
			m_pMotionBlurPS->AddConstantBuffer<CB_Globals>();

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
			RenderContext::SetDepthStencilEnabled( false );

			RenderTexture* src = data.GetRenderTexture( "src" );
			RenderTexture* dst = data.GetRenderTexture( "dst" );
			Texture* depth = data.GetTexture( "depth" );
			Camera* cam = scene.GetActiveCamera();

			int width = src->GetTextureWidth();
			int height = src->GetTextureHeight();

			CB_TexturePipelineMatrix transform;
			transform.viewproj = DirectX::XMMatrixOrthographicLH( (float)width, (float)height, Graphics::ScreenNear, Graphics::ScreenFar );
			transform.world = DirectX::XMMatrixIdentity();
			RenderContext::GetDeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

			CB_Globals psGlobals;
			psGlobals.resolution = { (float)width, (float)height };
			psGlobals.time = g_pGlobals->elapsed_time;
			DirectX::XMMATRIX viewproj = cam->GetViewMatrix() * cam->GetProjectionMatrix();
			psGlobals.inv_viewproj = DirectX::XMMatrixInverse( nullptr, viewproj );
			psGlobals.prev_viewproj = m_matPrevViewProj;

			m_pMotionBlurPS->GetConstantBuffer( 0 ).SetData( &psGlobals );

			m_bufPosition.Bind( 0 );
			m_bufUV.Bind( 1 );
			m_bufIndices.Bind();

			m_pTextureVS->Bind();
			m_pTextureVS->GetConstantBuffer( 0 ).SetData( &transform );

			dst->Bind( false );
			m_pMotionBlurPS->Bind();
			m_pMotionBlurPS->SetResource( 0, src->GetTextureView() );
			m_pMotionBlurPS->SetResource( 1, depth->GetTextureView() );

			RenderContext::GetDeviceContext()->DrawIndexed( m_bufIndices.GetIndexCount(), 0, 0 );

			// Unbind depth buffer
			m_pMotionBlurPS->SetResource( 1, nullptr );

			m_matPrevViewProj = viewproj;
		}
	private:
		int m_iBlurPasses = 1;

		DirectX::XMMATRIX m_matPrevViewProj = DirectX::XMMatrixIdentity();

		Resource<VertexShader> m_pTextureVS;
		Resource<PixelShader> m_pMotionBlurPS;

		VertexBuffer<Vec4> m_bufPosition;
		VertexBuffer<Vec2> m_bufUV;
		IndexBuffer m_bufIndices;
	};
}