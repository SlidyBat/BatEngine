#include "PCH.h"
#include "BloomPostProcess.h"

#include "Globals.h"
#include "TexturePipeline.h"
#include "WindowEvents.h"

namespace Bat
{
	struct CB_Globals
	{
		Vec2 resolution;
		float time;
		float pad;
	};

	BloomPostProcess::BloomPostProcess()
		:
		GenericPostProcess( L"Graphics/Shaders/TexturePS.hlsl" )
	{
		m_BloomFrameBuffer.Resize( g_pGfx->GetScreenWidth(), g_pGfx->GetScreenHeight() );
		m_BloomFrameBuffer2.Resize( g_pGfx->GetScreenWidth(), g_pGfx->GetScreenHeight() );
		ON_EVENT_DISPATCHED( [=]( const WindowResizeEvent* e )
		{
			m_BloomFrameBuffer.Resize( g_pGfx->GetScreenWidth(), g_pGfx->GetScreenHeight() );
			m_BloomFrameBuffer2.Resize( g_pGfx->GetScreenWidth(), g_pGfx->GetScreenHeight() );
		} );

		m_pBrightExtractPS = ResourceManager::GetPixelShader( "Graphics/Shaders/BrightExtractPS.hlsl" );
		m_pBrightExtractPS->AddConstantBuffer<CB_Globals>();
		m_pGaussBlurHorPS = ResourceManager::GetPixelShader( "Graphics/Shaders/GaussBlurHorPS.hlsl" );
		m_pGaussBlurHorPS->AddConstantBuffer<CB_Globals>();
		m_pGaussBlurVerPS = ResourceManager::GetPixelShader( "Graphics/Shaders/GaussBlurVerPS.hlsl" );
		m_pGaussBlurVerPS->AddConstantBuffer<CB_Globals>();

		m_pBloomShader = ResourceManager::GetPixelShader( "Graphics/Shaders/BloomPS.hlsl" );
	}

	void BloomPostProcess::Render( RenderTexture& pRenderTexture )
	{
		CB_TexturePipelineMatrix transform;
		transform.viewproj = g_pGfx->GetOrthoMatrix();
		transform.world = DirectX::XMMatrixIdentity();
		g_pGfx->GetDeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		CB_Globals psGlobals;
		psGlobals.resolution = { (float)g_pGfx->GetScreenWidth(), (float)g_pGfx->GetScreenHeight() };
		psGlobals.time = g_pGlobals->elapsed_time;

		m_PixelShader.GetConstantBuffer( 0 ).SetData( &psGlobals );
		m_pBrightExtractPS->GetConstantBuffer( 0 ).SetData( &psGlobals );
		m_pGaussBlurHorPS->GetConstantBuffer( 0 ).SetData( &psGlobals );
		m_pGaussBlurVerPS->GetConstantBuffer( 0 ).SetData( &psGlobals );

		m_bufPosition.Bind( 0 );
		m_bufUV.Bind( 1 );
		m_bufIndices.Bind();

		m_VertexShader.Bind();
		m_VertexShader.GetConstantBuffer( 0 ).SetData( &transform );

		m_BloomFrameBuffer.Bind();
		m_pBrightExtractPS->Bind();
		m_pBrightExtractPS->SetResource( 0, pRenderTexture.GetTextureView() );
		g_pGfx->GetDeviceContext()->DrawIndexed( m_bufIndices.GetIndexCount(), 0, 0 );

		for( int i = 0; i < 5 * 2; i++ )
		{
			if( i % 2 == 0 )
			{
				m_BloomFrameBuffer2.Bind();
				m_pGaussBlurHorPS->Bind();
				m_pGaussBlurHorPS->SetResource( 0, m_BloomFrameBuffer.GetTextureView() );
			}
			else
			{
				m_BloomFrameBuffer.Bind();
				m_pGaussBlurVerPS->Bind();
				m_pGaussBlurVerPS->SetResource( 0, m_BloomFrameBuffer2.GetTextureView() );
			}
			g_pGfx->GetDeviceContext()->DrawIndexed( m_bufIndices.GetIndexCount(), 0, 0 );
		}

		m_pBloomShader->Bind();
		m_pBloomShader->SetResource( 0, pRenderTexture.GetTextureView() );
		m_pBloomShader->SetResource( 1, m_BloomFrameBuffer2.GetTextureView() );
		auto pDeviceContext = g_pGfx->GetDeviceContext();
		auto pBackBuffer = g_pGfx->GetRenderTargetView();
		pDeviceContext->OMSetRenderTargets( 1, &pBackBuffer, g_pGfx->GetDepthStencilView() );
		g_pGfx->GetDeviceContext()->DrawIndexed( m_bufIndices.GetIndexCount(), 0, 0 );
	}
}