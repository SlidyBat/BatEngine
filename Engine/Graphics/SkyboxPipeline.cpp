#include "PCH.h"
#include "SkyboxPipeline.h"

#include "VertexTypes.h"
#include "COMException.h"

namespace Bat
{
	SkyboxPipeline::SkyboxPipeline( const std::string& vsFilename, const std::string& psFilename )
		:
		IPipeline( vsFilename, psFilename ),
		m_bufPositions({
			// Front Face
			Vec4( -1.0f, -1.0f, -1.0f, 1.0f ),
			Vec4( -1.0f,  1.0f, -1.0f, 1.0f ),
			Vec4(  1.0f,  1.0f, -1.0f, 1.0f ),
			Vec4(  1.0f, -1.0f, -1.0f, 1.0f ),

			// Back Face            
			Vec4( -1.0f, -1.0f, 1.0f, 1.0f ),
			Vec4(  1.0f, -1.0f, 1.0f, 1.0f ),
			Vec4(  1.0f,  1.0f, 1.0f, 1.0f ),
			Vec4( -1.0f,  1.0f, 1.0f, 1.0f ),

			// Top Face                
			Vec4( -1.0f, 1.0f, -1.0f, 1.0f ),
			Vec4( -1.0f, 1.0f,  1.0f, 1.0f ),
			Vec4(  1.0f, 1.0f,  1.0f, 1.0f ),
			Vec4(  1.0f, 1.0f, -1.0f, 1.0f ),

			// Bottom Face            
			Vec4( -1.0f, -1.0f, -1.0f, 1.0f ),
			Vec4(  1.0f, -1.0f, -1.0f, 1.0f ),
			Vec4(  1.0f, -1.0f,  1.0f, 1.0f ),
			Vec4( -1.0f, -1.0f,  1.0f, 1.0f ),

			// Left Face            
			Vec4( -1.0f, -1.0f,  1.0f, 1.0f ),
			Vec4( -1.0f,  1.0f,  1.0f, 1.0f ),
			Vec4( -1.0f,  1.0f, -1.0f, 1.0f ),
			Vec4( -1.0f, -1.0f, -1.0f, 1.0f ),

			// Right Face            
			Vec4( 1.0f, -1.0f, -1.0f, 1.0f ),
			Vec4( 1.0f,  1.0f, -1.0f, 1.0f ),
			Vec4( 1.0f,  1.0f,  1.0f, 1.0f ),
			Vec4( 1.0f, -1.0f,  1.0f, 1.0f ) }),
		m_bufIndices({
			// Front Face
			2,  1,  0,
			3,  2,  0,

			// Back Face
			6,  5,  4,
			7,  6,  4,

			// Top Face
			10,  9, 8,
			11, 10, 8,

			// Bottom Face
			14, 13, 12,
			15, 14, 12,

			// Left Face
			18, 17, 16,
			19, 18, 16,

			// Right Face
			22, 21, 20,
			23, 22, 20 })
	{}

	void SkyboxPipeline::BindParameters( IGPUContext* pContext, IPipelineParameters& pParameters )
	{
		auto params = static_cast<SkyboxPipelineParameters&>(pParameters);

		CB_SkyboxPipelineMatrix transform;
		transform.viewproj = params.transform;
		m_cbufTransform.Update( pContext, transform );

		pContext->SetVertexBuffer( m_bufPositions, 0 );
		pContext->SetIndexBuffer( m_bufIndices );
		pContext->SetVertexShader( m_pVertexShader.get() );
		pContext->SetPixelShader( m_pPixelShader.get() );
		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, 0 );
		pContext->BindTexture( params.texture, 0 );
	}

	void SkyboxPipeline::Render( IGPUContext* pContext, size_t vertexcount )
	{
		ASSERT( false, "Skybox does not support non-indexed rendering" );
	}

	void SkyboxPipeline::RenderIndexed( IGPUContext* pContext, size_t indexcount )
	{
		pContext->SetPrimitiveTopology( PrimitiveTopology::TRIANGLELIST );
		pContext->DrawIndexed( m_bufIndices->GetIndexCount() );
	}
}