#include "PCH.h"
#include "ColourPipeline.h"

#include "RenderContext.h"
#include "VertexTypes.h"
#include "COMException.h"
#include "RenderContext.h"

namespace Bat
{
	ColourPipeline::ColourPipeline( const std::string& vsFilename, const std::string& psFilename )
		:
		IPipeline( vsFilename, psFilename )
	{
		m_VertexShader.AddConstantBuffer<CB_ColourPipelineMatrix>();
	}

	void ColourPipeline::BindParameters( IPipelineParameters& pParameters )
	{
		auto pColourParameters = static_cast<ColourPipelineParameters&>(pParameters);
		m_VertexShader.Bind();
		m_PixelShader.Bind();
		m_VertexShader.GetConstantBuffer( 0 ).SetData( pColourParameters.GetTransformMatrix() );
	}

	void ColourPipeline::Render( UINT vertexcount )
	{
		auto pDeviceContext = RenderContext::GetDeviceContext();
		pDeviceContext->Draw( vertexcount, 0 );
	}

	void ColourPipeline::RenderIndexed( UINT indexcount )
	{
		auto pDeviceContext = RenderContext::GetDeviceContext();
		pDeviceContext->DrawIndexed( indexcount, 0, 0 );
	}
}