#include "PCH.h"

#include "Camera.h"
#include "BumpMapPipeline.h"
#include "VertexTypes.h"
#include "COMException.h"
#include "Graphics.h"
#include "Globals.h"
#include "Light.h"
#include "Material.h"

namespace Bat
{
	BumpMapPipeline::BumpMapPipeline( const std::wstring& vsFilename, const std::wstring& psFilename )
		:
		LightPipeline( vsFilename, psFilename )
	{}

	void BumpMapPipeline::BindParameters( IPipelineParameters& pParameters )
	{
		auto pTextureParameters = static_cast<LightPipelineParameters&>(pParameters);

		CB_LightPipelineLightingParams ps_params;
		ps_params.cameraPos = pTextureParameters.camera.GetPosition();
		ps_params.time = g_pGlobals->elapsed_time;
		ps_params.shininess = pTextureParameters.material.GetShininess();

		const float time = g_pGlobals->elapsed_time;
		CB_LightPipelineLight ps_light;
		ps_light.lightPos = m_pLight->GetPosition();
		ps_light.lightAmbient = m_pLight->GetAmbient();
		ps_light.lightDiffuse = m_pLight->GetDiffuse();
		ps_light.lightSpecular = m_pLight->GetSpecular();

		m_VertexShader.Bind();
		m_PixelShader.Bind();
		m_VertexShader.GetConstantBuffer( 0 ).SetData( &pTextureParameters.transform );
		m_PixelShader.GetConstantBuffer( 0 ).SetData( &ps_params );
		m_PixelShader.GetConstantBuffer( 1 ).SetData( &ps_light );

		ASSERT( pTextureParameters.material.GetDiffuseTexture(), "Material doesn't have diffuse texture" );
		ASSERT( pTextureParameters.material.GetSpecularTexture(), "Material doesn't have specular texture" );
		ASSERT( pTextureParameters.material.GetEmissiveTexture(), "Material doesn't have emissive texture" );
		ASSERT( pTextureParameters.material.GetBumpMapTexture(), "Material doesn't have bump map" );

		ID3D11ShaderResourceView* pTextures[] = {
			pTextureParameters.material.GetDiffuseTexture()->GetTextureView(),
			pTextureParameters.material.GetSpecularTexture()->GetTextureView(),
			pTextureParameters.material.GetEmissiveTexture()->GetTextureView(),
			pTextureParameters.material.GetBumpMapTexture()->GetTextureView()
		};

		m_PixelShader.SetResources( 0, pTextures, 4 );
	}
}