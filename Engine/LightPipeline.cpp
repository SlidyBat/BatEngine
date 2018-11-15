#include "LightPipeline.h"
#include "VertexTypes.h"
#include <fstream>
#include "COMException.h"
#include "IGraphics.h"

namespace Bat
{
	LightModel::LightModel( Mesh mesh )
	{
		m_Meshes.emplace_back( mesh );
	}
	LightModel::LightModel( std::vector<Mesh> meshes )
		:
		m_Meshes( std::move( meshes ) )
	{}

	void LightModel::Draw( IPipeline* pPipeline ) const
	{
		auto vp = DirectX::XMMatrixTranspose( g_pGfx->GetCamera()->GetViewMatrix() * g_pGfx->GetCamera()->GetProjectionMatrix() );
		auto w = DirectX::XMMatrixTranspose( GetWorldMatrix() );

		for( const auto& mesh : m_Meshes )
		{
			mesh.Bind();
			LightPipelineParameters params( w, vp, mesh.GetTexture() );
			pPipeline->BindParameters( &params );
			pPipeline->RenderIndexed( (UINT)mesh.GetIndexCount() );
		}
	}

	LightPipeline::LightPipeline( const std::wstring& vsFilename, const std::wstring& psFilename )
		:
		m_VertexShader( vsFilename, Vertex::InputLayout, Vertex::Inputs ),
		m_PixelShader( psFilename )
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

		m_VertexShader.AddConstantBuffer<CB_LightPipelineMatrix>();
	}

	void LightPipeline::BindParameters( IPipelineParameters* pParameters )
	{
		auto pTextureParameters = static_cast<LightPipelineParameters*>(pParameters);
		m_VertexShader.Bind();
		m_PixelShader.Bind();
		m_VertexShader.GetConstantBuffer( 0 ).SetData( pTextureParameters->GetTransformMatrix() );
		m_PixelShader.SetResource( 0, pTextureParameters->GetTextureView() );
	}

	void LightPipeline::Render( UINT vertexcount )
	{
		auto pDeviceContext = g_pGfx->GetDeviceContext();
		pDeviceContext->Draw( vertexcount, 0 );
	}

	void LightPipeline::RenderIndexed( UINT indexcount )
	{
		auto pDeviceContext = g_pGfx->GetDeviceContext();
		pDeviceContext->DrawIndexed( indexcount, 0, 0 );
	}
}