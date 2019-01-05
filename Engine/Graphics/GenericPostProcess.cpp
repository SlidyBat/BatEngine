#include "PCH.h"
#include "GenericPostProcess.h"

#include "RenderContext.h"
#include "TexturePipeline.h"
#include "RenderTexture.h"
#include "Globals.h"

namespace Bat
{
	struct CB_Globals
	{
		Vec2 resolution;
		float time;
		float pad;
	};

	GenericPostProcess::GenericPostProcess( int width, int height, const std::string& psFilename )
		:
		m_VertexShader( "Graphics/Shaders/TextureVS.hlsl" ),
		m_PixelShader( psFilename )
	{
		m_VertexShader.AddConstantBuffer<CB_TexturePipelineMatrix>();

		m_PixelShader.AddConstantBuffer<CB_Globals>();

		const float fWidth =  (float)width;
		const float fHeight = (float)height;

		const float left = -fWidth / 2;
		const float right = fWidth / 2;
		const float top = fHeight / 2;
		const float bottom = -fHeight / 2;

		const std::vector<Vec4> positions = {
			{ left, bottom, 1.0f, 1.0f },
			{ left, top, 1.0f, 1.0f },
			{ right, top, 1.0f, 1.0f },
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

	void GenericPostProcess::Render( RenderTexture& pRenderTexture )
	{
		const float width =  (float)pRenderTexture.GetTextureWidth();
		const float height = (float)pRenderTexture.GetTextureHeight();

		CB_TexturePipelineMatrix transform;
		transform.viewproj = DirectX::XMMatrixOrthographicLH( width, height, 0.1f, 1000.0f );
		transform.world = DirectX::XMMatrixIdentity();
		RenderContext::GetDeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		CB_Globals psGlobals;
		psGlobals.resolution = { width, height };
		psGlobals.time = g_pGlobals->elapsed_time;

		m_bufPosition.Bind( 0 );
		m_bufUV.Bind( 1 );
		m_bufIndices.Bind();

		m_VertexShader.Bind();
		m_PixelShader.Bind();
		m_VertexShader.GetConstantBuffer( 0 ).SetData( &transform );
		m_PixelShader.SetResource( 0, pRenderTexture.GetTextureView() );
		m_PixelShader.GetConstantBuffer( 0 ).SetData( &psGlobals );

		RenderContext::GetDeviceContext()->DrawIndexed( m_bufIndices.GetIndexCount(), 0, 0 );
	}
}
