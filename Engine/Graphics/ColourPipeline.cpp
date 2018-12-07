#include "PCH.h"
#include "ColourPipeline.h"

#include "VertexTypes.h"
#include "COMException.h"
#include "IGraphics.h"

namespace Bat
{
	ColouredModel::ColouredModel( const Mesh& mesh )
	{
		m_Meshes.emplace_back( mesh );
	}
	ColouredModel::ColouredModel( std::vector<Mesh> meshes )
		:
		m_Meshes( std::move( meshes ) )
	{}

	void ColouredModel::Draw( IPipeline* pPipeline ) const
	{
		auto vp = g_pGfx->GetCamera()->GetViewMatrix() * g_pGfx->GetCamera()->GetProjectionMatrix();
		auto w = GetWorldMatrix();

		for( const auto& mesh : m_Meshes )
		{
			mesh.Bind( pPipeline );
			ColourPipelineParameters params( w, vp );
			pPipeline->BindParameters( &params );
			pPipeline->RenderIndexed( (UINT)mesh.GetIndexCount() );
		}
	}

	ColourPipeline::ColourPipeline( const std::wstring& vsFilename, const std::wstring& psFilename )
		:
		IPipeline( vsFilename, psFilename )
	{
		m_VertexShader.AddConstantBuffer<CB_ColourPipelineMatrix>();
	}

	void ColourPipeline::BindParameters( IPipelineParameters* pParameters )
	{
		auto pColourParameters = static_cast<ColourPipelineParameters*>(pParameters);
		m_VertexShader.Bind();
		m_PixelShader.Bind();
		m_VertexShader.GetConstantBuffer( 0 ).SetData( pColourParameters->GetTransformMatrix() );
	}

	void ColourPipeline::Render( UINT vertexcount )
	{
		auto pDeviceContext = g_pGfx->GetDeviceContext();
		pDeviceContext->Draw( vertexcount, 0 );
	}

	void ColourPipeline::RenderIndexed( UINT indexcount )
	{
		auto pDeviceContext = g_pGfx->GetDeviceContext();
		pDeviceContext->DrawIndexed( indexcount, 0, 0 );
	}
}