#include "PCH.h"
#include "LightPipeline.h"

#include "Camera.h"
#include "VertexTypes.h"
#include "COMException.h"
#include "RenderContext.h"
#include "Globals.h"
#include "Light.h"
#include "Material.h"

namespace Bat
{
	LightPipeline::LightPipeline( const std::string& vsFilename, const std::string& psFilename )
		:
		IPipeline( vsFilename, psFilename )
	{
		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		m_PixelShader.AddSampler( &samplerDesc );

		m_VertexShader.AddConstantBuffer<CB_LightPipelineMatrix>();
		m_PixelShader.AddConstantBuffer<CB_LightPipelineLightingParams>();
		m_PixelShader.AddConstantBuffer<CB_LightPipelineLight>();
	}

	void LightPipeline::BindParameters( IPipelineParameters& pParameters )
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

		ID3D11ShaderResourceView* pTextures[] = {
			pTextureParameters.material.GetDiffuseTexture()->GetTextureView(),
			pTextureParameters.material.GetSpecularTexture()->GetTextureView(),
			pTextureParameters.material.GetEmissiveTexture()->GetTextureView()
		};

		m_PixelShader.SetResources( 0, pTextures, 3 );
	}

	void LightPipeline::Render( UINT vertexcount )
	{
		auto pDeviceContext = RenderContext::GetDeviceContext();
		pDeviceContext->Draw( vertexcount, 0 );
	}

	void LightPipeline::RenderIndexed( UINT indexcount )
	{
		auto pDeviceContext = RenderContext::GetDeviceContext();
		pDeviceContext->DrawIndexed( indexcount, 0, 0 );
	}
}