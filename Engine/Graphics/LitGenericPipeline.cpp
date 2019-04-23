#include "PCH.h"

#include "Camera.h"
#include "LitGenericPipeline.h"
#include "VertexTypes.h"
#include "COMException.h"
#include "Graphics.h"
#include "Globals.h"
#include "Texture.h"
#include "Light.h"
#include "Material.h"
#include "ShaderManager.h"

namespace Bat
{
	LitGenericPipeline::LitGenericPipeline( const std::string& vs_filename, const std::string& ps_filename )
		:
		IPipeline( vs_filename, ps_filename )
	{}

	void LitGenericPipeline::BindParameters( IGPUContext* pContext, IPipelineParameters& pParameters )
	{
		auto params = static_cast<LitGenericPipelineParameters&>(pParameters);

		CB_LitGenericPipelineMatrix transform;
		transform.world = params.world;
		transform.viewproj = params.viewproj;
		m_cbufTransform.Update( pContext, transform );
		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, 0 );

		CB_LitGenericPipelineMaterial material;
		material.material.GlobalAmbient = { 1.0f, 1.0f, 1.0f };
		material.material.AmbientColor = { 0.0f, 0.0f, 0.0f };
		material.material.DiffuseColor = { 0.0f, 0.0f, 0.0f };
		material.material.SpecularColor = { 0.0f, 0.0f, 0.0f };
		material.material.EmissiveColor = { 0.0f, 0.0f, 0.0f };

		material.material.HasAmbientTexture  = (params.material.GetAmbientTexture() != nullptr);
		material.material.HasDiffuseTexture  = (params.material.GetDiffuseTexture() != nullptr);
		material.material.HasSpecularTexture = (params.material.GetSpecularTexture() != nullptr);
		material.material.HasEmissiveTexture = (params.material.GetEmissiveTexture() != nullptr);
		material.material.HasNormalTexture   = (params.material.GetNormalTexture() != nullptr);
		material.material.HasBumpTexture     = false;
		material.material.SpecularPower      = params.material.GetShininess();
		m_cbufMaterial.Update( pContext, material );
		pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufMaterial, PS_CBUF_SLOT_0 );

		CB_LitGenericPipelineLights lights;
		size_t num_lights = std::min( MAX_LIGHTS, params.lights.size() );
		lights.num_lights = (uint32_t)num_lights;
		for( size_t i = 0; i < num_lights; i++ )
		{
			lights.lights[i].Position = params.lights[i]->GetPosition();
			lights.lights[i].Diffuse = params.lights[i]->GetDiffuse();
			lights.lights[i].Specular = params.lights[i]->GetSpecular();
			lights.lights[i].Range = params.lights[i]->GetRange();
			lights.lights[i].Type = 0; // TODO: implement more light types;
		}
		m_cbufLightParams.Update( pContext, lights );
		pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufLightParams, PS_CBUF_SLOT_1 );

		pContext->SetVertexShader( m_pVertexShader.get() );
		pContext->SetPixelShader( m_pPixelShader.get() );

		if( auto tex = params.material.GetDiffuseTexture() )  pContext->BindTexture( *tex, PS_TEX_SLOT_0 );
		if( auto tex = params.material.GetSpecularTexture() ) pContext->BindTexture( *tex, PS_TEX_SLOT_1 );
		if( auto tex = params.material.GetEmissiveTexture() ) pContext->BindTexture( *tex, PS_TEX_SLOT_2 );
		if( auto tex = params.material.GetNormalTexture() )   pContext->BindTexture( *tex, PS_TEX_SLOT_3 );
		if( auto tex = params.material.GetAmbientTexture() )  pContext->BindTexture( *tex, PS_TEX_SLOT_4 );
	}

	void LitGenericPipeline::Render( IGPUContext* pContext, size_t vertexcount )
	{
		pContext->Draw( vertexcount );
	}

	void LitGenericPipeline::RenderIndexed( IGPUContext* pContext, size_t indexcount )
	{
		pContext->DrawIndexed( indexcount );
	}
}