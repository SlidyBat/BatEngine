#include "PCH.h"
#include "TexturePipeline.h"

#include "VertexTypes.h"
#include "COMException.h"
#include "Material.h"

namespace Bat
{
	TexturePipeline::TexturePipeline( const std::string& vs_filename, const std::string& ps_filename )
		:
		IPipeline( vs_filename, ps_filename )
	{}

	void TexturePipeline::BindParameters( IGPUContext* pContext, IPipelineParameters& pParameters )
	{
		auto params = static_cast<TexturePipelineParameters&>(pParameters);

		CB_TexturePipelineMatrix transform;
		transform.world = params.transform.world;
		transform.viewproj = params.transform.viewproj;
		m_cbufTransform.Update( pContext, transform );
		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, 0 );

		pContext->SetVertexShader( m_pVertexShader.get() );
		pContext->SetPixelShader( m_pPixelShader.get() );
		pContext->BindTexture( params.texture, 0 );
	}

	void TexturePipeline::Render( IGPUContext* pContext, size_t vertexcount )
	{
		pContext->Draw( vertexcount );
	}

	void TexturePipeline::RenderIndexed( IGPUContext* pContext, size_t indexcount )
	{
		pContext->DrawIndexed( indexcount );
	}
}