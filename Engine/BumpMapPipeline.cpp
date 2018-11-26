#include "BumpMapPipeline.h"
#include "VertexTypes.h"
#include <fstream>
#include "COMException.h"
#include "IGraphics.h"
#include "Globals.h"
#include "Light.h"
#include "Material.h"

namespace Bat
{
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
		m_PixelShader.SetResource( 0, pTextureParameters->material->GetDiffuseTexture()->GetTextureView() );
		ASSERT( pTextureParameters->material->GetSpecularTexture(), "Material doesn't have specular texture" );
		m_PixelShader.SetResource( 1, pTextureParameters->material->GetSpecularTexture()->GetTextureView() );
		ASSERT( pTextureParameters->material->GetAmbientTexture(), "Material doesn't have ambient texture" );
		m_PixelShader.SetResource( 2, pTextureParameters->material->GetAmbientTexture()->GetTextureView() );
		ASSERT( pTextureParameters->material->GetBumpMapTexture(), "Material doesn't have bump map" );
		m_PixelShader.SetResource( 3, pTextureParameters->material->GetBumpMapTexture()->GetTextureView() );
	}
}