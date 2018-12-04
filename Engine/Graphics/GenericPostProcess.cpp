#include "GenericPostProcess.h"

#include "IGraphics.h"
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

	GenericPostProcess::GenericPostProcess( const std::wstring& psFilename )
		:
		m_VertexShader( L"Graphics/Shaders/Build/TextureVS.cso" ),
		m_PixelShader( psFilename )
	{
		m_VertexShader.AddConstantBuffer<CB_TexturePipelineMatrix>();

		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		m_PixelShader.AddSampler( &samplerDesc );
		m_PixelShader.AddConstantBuffer<CB_Globals>();

		const float width = (float)g_pGfx->GetScreenWidth();
		const float height = (float)g_pGfx->GetScreenHeight();

		const float left = -width / 2;
		const float right = width / 2;
		const float top = height / 2;
		const float bottom = -height / 2;

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

	void GenericPostProcess::Render( ID3D11ShaderResourceView* pTexture )
	{
		CB_TexturePipelineMatrix transform;
		transform.viewproj = g_pGfx->GetOrthoMatrix();
		transform.world = DirectX::XMMatrixIdentity();
		g_pGfx->GetDeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		CB_Globals psGlobals;
		psGlobals.resolution = { (float)g_pGfx->GetScreenWidth(), (float)g_pGfx->GetScreenHeight() };
		psGlobals.time = g_pGlobals->elapsed_time;

		m_bufPosition.Bind( 0 );
		m_bufUV.Bind( 1 );
		m_bufIndices.Bind();

		m_VertexShader.Bind();
		m_PixelShader.Bind();
		m_VertexShader.GetConstantBuffer( 0 ).SetData( &transform );
		m_PixelShader.SetResource( 0, pTexture );
		m_PixelShader.GetConstantBuffer( 0 ).SetData( &psGlobals );

		g_pGfx->GetDeviceContext()->DrawIndexed( m_bufIndices.GetIndexCount(), 0, 0 );
	}
}
