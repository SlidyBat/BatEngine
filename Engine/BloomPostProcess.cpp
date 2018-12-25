#include "PCH.h"
#include "BloomPostProcess.h"

#include "Globals.h"
#include "TexturePipeline.h"
#include "WindowEvents.h"
#include "RenderContext.h"

namespace Bat
{
	struct CB_Globals
	{
		Vec2 resolution;
		float time;
		float pad;
	};

	BloomPostProcess::BloomPostProcess( Window& wnd )
		:
		GenericPostProcess( wnd.GetWidth(), wnd.GetHeight(), "Graphics/Shaders/TexturePS.hlsl" ),
		m_iWidth( wnd.GetWidth() ),
		m_iHeight( wnd.GetHeight() )
	{
		const int width = wnd.GetWidth();
		const int height = wnd.GetHeight();

		m_BloomFrameBuffer.Resize( wnd.GetWidth(), height );
		m_BloomFrameBuffer2.Resize( width, height );

		m_pBrightExtractPS = ResourceManager::GetPixelShader( "Graphics/Shaders/BrightExtractPS.hlsl" );
		m_pBrightExtractPS->AddConstantBuffer<CB_Globals>();
		m_pGaussBlurHorPS = ResourceManager::GetPixelShader( "Graphics/Shaders/GaussBlurHorPS.hlsl" );
		m_pGaussBlurHorPS->AddConstantBuffer<CB_Globals>();
		m_pGaussBlurVerPS = ResourceManager::GetPixelShader( "Graphics/Shaders/GaussBlurVerPS.hlsl" );
		m_pGaussBlurVerPS->AddConstantBuffer<CB_Globals>();

		m_pBloomShader = ResourceManager::GetPixelShader( "Graphics/Shaders/BloomPS.hlsl" );

		wnd.OnEventDispatched<WindowResizeEvent>( [=]( const WindowResizeEvent& e )
		{
			m_iWidth = e.GetWidth();
			m_iHeight = e.GetHeight();
			m_BloomFrameBuffer.Resize( m_iWidth, m_iHeight );
			m_BloomFrameBuffer2.Resize( m_iWidth, m_iHeight );
		} );
	}

	void BloomPostProcess::Render( RenderTexture& pRenderTexture )
	{
		CB_TexturePipelineMatrix transform;
		transform.viewproj = DirectX::XMMatrixOrthographicLH( (float)m_iWidth, (float)m_iHeight, 0.1f, 1000.0f );
		transform.world = DirectX::XMMatrixIdentity();
		RenderContext::GetDeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		CB_Globals psGlobals;
		psGlobals.resolution = { (float)m_iWidth, (float)m_iHeight };
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
		RenderContext::GetDeviceContext()->DrawIndexed( m_bufIndices.GetIndexCount(), 0, 0 );

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
			RenderContext::GetDeviceContext()->DrawIndexed( m_bufIndices.GetIndexCount(), 0, 0 );
		}

		m_pBloomShader->Bind();
		m_pBloomShader->SetResource( 0, pRenderTexture.GetTextureView() );
		m_pBloomShader->SetResource( 1, m_BloomFrameBuffer2.GetTextureView() );
		auto pDeviceContext = RenderContext::GetDeviceContext();
		auto pBackBuffer = RenderContext::GetBackBufferView();
		pDeviceContext->OMSetRenderTargets( 1, &pBackBuffer, RenderContext::GetDepthStencilView() );
		RenderContext::GetDeviceContext()->DrawIndexed( m_bufIndices.GetIndexCount(), 0, 0 );
	}
}