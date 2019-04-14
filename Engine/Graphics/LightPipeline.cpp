#include "PCH.h"
#include "LightPipeline.h"

#include "Camera.h"
#include "VertexTypes.h"
#include "COMException.h"
#include "Globals.h"
#include "Light.h"
#include "Material.h"

namespace Bat
{
	LightPipeline::LightPipeline( const std::string& vs_filename, const std::string& ps_filename )
		:
		IPipeline( vs_filename, ps_filename )
	{}

	void LightPipeline::BindParameters( IGPUContext* pContext, IPipelineParameters& pParameters )
	{
		auto params = static_cast<LightPipelineParameters&>(pParameters);

		CB_LightPipelineLightingParams ps_params;
		ps_params.cameraPos = params.camera.GetPosition();
		ps_params.time = g_pGlobals->elapsed_time;
		ps_params.shininess = params.material.GetShininess();
		m_cbufLightParams.Update( pContext, ps_params );
		pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufLightParams, 0 );

		const float time = g_pGlobals->elapsed_time;
		CB_LightPipelineLight ps_light;
		if( m_pLight )
		{
			ps_light.lightPos = m_pLight->GetPosition();
			ps_light.lightAmbient = m_pLight->GetAmbient();
			ps_light.lightDiffuse = m_pLight->GetDiffuse();
			ps_light.lightSpecular = m_pLight->GetSpecular();
		}
		m_cbufLight.Update( pContext, ps_light );
		pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufLight, 1 );

		m_cbufTransform.Update( pContext, params.transform );
		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, 0 );

		pContext->SetVertexShader( m_pVertexShader.get() );
		pContext->SetPixelShader( m_pPixelShader.get() );

		ASSERT( params.material.GetDiffuseTexture(), "Material doesn't have diffuse texture" );
		ASSERT( params.material.GetSpecularTexture(), "Material doesn't have specular texture" );
		ASSERT( params.material.GetEmissiveTexture(), "Material doesn't have emissive texture" );

		pContext->BindTexture( *params.material.GetDiffuseTexture(),  0 );
		pContext->BindTexture( *params.material.GetSpecularTexture(), 1 );
		pContext->BindTexture( *params.material.GetEmissiveTexture(), 2 );
	}

	void LightPipeline::Render( IGPUContext* pContext, size_t vertexcount )
	{
		pContext->Draw( vertexcount );
	}

	void LightPipeline::RenderIndexed( IGPUContext* pContext, size_t indexcount )
	{
		pContext->DrawIndexed( indexcount );
	}
}