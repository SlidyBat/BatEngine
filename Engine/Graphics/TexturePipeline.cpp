#include "PCH.h"
#include "TexturePipeline.h"

#include "VertexTypes.h"
#include "COMException.h"
#include "RenderContext.h"
#include "Material.h"

namespace Bat
{
	TexturePipeline::TexturePipeline( const std::string& vsFilename, const std::string& psFilename )
		:
		IPipeline( vsFilename, psFilename )
	{
		m_VertexShader.AddConstantBuffer<CB_TexturePipelineMatrix>();
	}

	void TexturePipeline::BindParameters( IPipelineParameters& pParameters )
	{
		auto pTextureParameters = static_cast<TexturePipelineParameters&>(pParameters);
		m_VertexShader.Bind();
		m_PixelShader.Bind();
		m_VertexShader.GetConstantBuffer( 0 ).SetData( &pTextureParameters.transform );
		m_PixelShader.SetResource( 0, pTextureParameters.texture );
	}

	void TexturePipeline::Render( UINT vertexcount )
	{
		auto pDeviceContext = RenderContext::GetDeviceContext();
		pDeviceContext->Draw( vertexcount, 0 );
	}

	void TexturePipeline::RenderIndexed( UINT indexcount )
	{
		auto pDeviceContext = RenderContext::GetDeviceContext();
		pDeviceContext->DrawIndexed( indexcount, 0, 0 );
	}
}