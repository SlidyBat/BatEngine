#include "PCH.h"

#include "BumpMapPipeline.h"
#include "VertexTypes.h"
#include "COMException.h"
#include "IGraphics.h"
#include "Globals.h"
#include "Light.h"
#include "Material.h"

namespace Bat
{
	BumpMappedModel::BumpMappedModel( const Mesh& mesh )
	{
		m_Meshes.emplace_back( mesh );
	}

	BumpMappedModel::BumpMappedModel( std::vector<Mesh> meshes )
		:
		m_Meshes( std::move( meshes ) )
	{}

	void BumpMappedModel::Draw( IPipeline* pPipeline ) const
	{
		auto vp = g_pGfx->GetCamera()->GetViewMatrix() * g_pGfx->GetCamera()->GetProjectionMatrix();
		auto w = GetWorldMatrix();

		for( const auto& mesh : m_Meshes )
		{
			LightPipelineParameters params( mesh.GetTransform() * w, vp, mesh.GetMaterial() );

			if( !mesh.GetMaterial()->GetBumpMapTexture() )
			{
				// cant render this mesh, fall back to light pipeline
				auto pLightPipeline = static_cast<LightPipeline*>(g_pGfx->GetPipeline( "light" ));
				pLightPipeline->SetLight( static_cast<LightPipeline*>(pPipeline)->GetLight() );
				mesh.Bind( pLightPipeline );
				pLightPipeline->BindParameters( &params );
				pLightPipeline->RenderIndexed( (UINT)mesh.GetIndexCount() );
			}
			else
			{
				mesh.Bind( pPipeline );
				pPipeline->BindParameters( &params );
				pPipeline->RenderIndexed( (UINT)mesh.GetIndexCount() );
			}
		}
	}

	BumpMapPipeline::BumpMapPipeline( const std::wstring& vsFilename, const std::wstring& psFilename )
		:
		LightPipeline( vsFilename, psFilename )
	{}

	void BumpMapPipeline::BindParameters( IPipelineParameters* pParameters )
	{
		auto pTextureParameters = static_cast<LightPipelineParameters*>(pParameters);

		CB_LightPipelineLightingParams ps_params;
		ps_params.cameraPos = g_pGfx->GetCamera()->GetPosition();
		ps_params.time = g_pGlobals->elapsed_time;
		ps_params.shininess = pTextureParameters->material->GetShininess();

		const float time = g_pGlobals->elapsed_time;
		CB_LightPipelineLight ps_light;
		ps_light.lightPos = m_pLight->GetPosition();
		ps_light.lightAmbient = m_pLight->GetAmbient();
		ps_light.lightDiffuse = m_pLight->GetDiffuse();
		ps_light.lightSpecular = m_pLight->GetSpecular();

		m_VertexShader.Bind();
		m_PixelShader.Bind();
		m_VertexShader.GetConstantBuffer( 0 ).SetData( &pTextureParameters->transform );
		m_PixelShader.GetConstantBuffer( 0 ).SetData( &ps_params );
		m_PixelShader.GetConstantBuffer( 1 ).SetData( &ps_light );

		ASSERT( pTextureParameters->material->GetDiffuseTexture(), "Material doesn't have diffuse texture" );
		ASSERT( pTextureParameters->material->GetSpecularTexture(), "Material doesn't have specular texture" );
		ASSERT( pTextureParameters->material->GetEmissiveTexture(), "Material doesn't have emissive texture" );
		ASSERT( pTextureParameters->material->GetBumpMapTexture(), "Material doesn't have bump map" );

		ID3D11ShaderResourceView* pTextures[] = {
			pTextureParameters->material->GetDiffuseTexture()->GetTextureView(),
			pTextureParameters->material->GetSpecularTexture()->GetTextureView(),
			pTextureParameters->material->GetEmissiveTexture()->GetTextureView(),
			pTextureParameters->material->GetBumpMapTexture()->GetTextureView()
		};

		m_PixelShader.SetResources( 0, pTextures, 4 );
	}
}