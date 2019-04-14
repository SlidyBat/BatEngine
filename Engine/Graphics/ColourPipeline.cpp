#include "PCH.h"
#include "ColourPipeline.h"

namespace Bat
{
	ColourPipeline::ColourPipeline( const std::string& vs_filename, const std::string& ps_filename )
		:
		IPipeline( vs_filename, ps_filename )
	{}

	void ColourPipeline::BindParameters( IGPUContext* pContext, IPipelineParameters& pParameters )
	{
		auto params = static_cast<ColourPipelineParameters&>(pParameters);

		m_cbufTransform.Update( pContext, params.transform );
		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, 0 );

		pContext->SetVertexShader( m_pVertexShader.get() );
		pContext->SetPixelShader( m_pPixelShader.get() );
	}

	void ColourPipeline::Render( IGPUContext* pContext, size_t vertexcount )
	{
		pContext->Draw( vertexcount );
	}

	void ColourPipeline::RenderIndexed( IGPUContext* pContext, size_t indexcount )
	{
		pContext->DrawIndexed( indexcount );
	}
}