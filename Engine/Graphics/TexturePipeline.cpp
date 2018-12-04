#include "TexturePipeline.h"
#include "VertexTypes.h"
#include <fstream>
#include "COMException.h"
#include "IGraphics.h"
#include "Material.h"

namespace Bat
{
	TexturedModel::TexturedModel( const Mesh& mesh )
	{
		m_Meshes.emplace_back( mesh );
	}
	TexturedModel::TexturedModel( std::vector<Mesh> meshes )
		:
		m_Meshes( std::move( meshes ) )
	{}

	void TexturedModel::Draw( IPipeline* pPipeline ) const
	{
		auto vp = g_pGfx->GetCamera()->GetViewMatrix() * g_pGfx->GetCamera()->GetProjectionMatrix();
		auto w = GetWorldMatrix();

		for( const auto& mesh : m_Meshes )
		{
			mesh.Bind( pPipeline );
			TexturePipelineParameters params( w, vp, mesh.GetMaterial()->GetDiffuseTexture()->GetTextureView() );
			pPipeline->BindParameters( &params );
			pPipeline->RenderIndexed( (UINT)mesh.GetIndexCount() );
		}
	}


	ScreenQuadModel::ScreenQuadModel( ID3D11ShaderResourceView* pTexture )
		:
		m_pTexture( pTexture )
	{
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

		MeshParameters params;
		params.position = positions;
		params.uv = uvs;
		m_Mesh.SetData( params );
		m_Mesh.SetIndices( indices );
	}

	void ScreenQuadModel::Draw( IPipeline* pPipeline ) const
	{
		auto vp = g_pGfx->GetOrthoMatrix();
		auto w = DirectX::XMMatrixIdentity();

		m_Mesh.Bind( pPipeline );
		TexturePipelineParameters params( w, vp, m_pTexture );
		pPipeline->BindParameters( &params );
		pPipeline->RenderIndexed( (UINT)m_Mesh.GetIndexCount() );
	}

	TexturePipeline::TexturePipeline( const std::wstring& vsFilename, const std::wstring& psFilename )
		:
		IPipeline( vsFilename, psFilename )
	{
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

		m_VertexShader.AddConstantBuffer<CB_TexturePipelineMatrix>();
	}

	void TexturePipeline::BindParameters( IPipelineParameters* pParameters )
	{
		auto pTextureParameters = static_cast<TexturePipelineParameters*>(pParameters);
		m_VertexShader.Bind();
		m_PixelShader.Bind();
		m_VertexShader.GetConstantBuffer( 0 ).SetData( &pTextureParameters->transform );
		m_PixelShader.SetResource( 0, pTextureParameters->texture );
	}

	void TexturePipeline::Render( UINT vertexcount )
	{
		auto pDeviceContext = g_pGfx->GetDeviceContext();
		pDeviceContext->Draw( vertexcount, 0 );
	}

	void TexturePipeline::RenderIndexed( UINT indexcount )
	{
		auto pDeviceContext = g_pGfx->GetDeviceContext();
		pDeviceContext->DrawIndexed( indexcount, 0, 0 );
	}
}