#pragma once

#include "PCH.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>

#include "IPipeline.h"
#include "Texture.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Mesh.h"

namespace Bat
{
	class Light;
	class Material;

	struct CB_LightPipelineMatrix
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX viewproj;
	};

	struct CB_LightPipelineLightingParams
	{
		Vec3 cameraPos;
		float time;
		float shininess;
	};

	struct CB_LightPipelineLight
	{
		Vec3 lightPos;
		float pad1;
		Vec3 lightAmbient;
		float pad2;
		Vec3 lightDiffuse;
		float pad3;
		Vec3 lightSpecular;
		float pad4;
	};

	class LightPipelineParameters : public IPipelineParameters
	{
	public:
		LightPipelineParameters( const Camera& camera, const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& viewproj, Material& pMaterial )
			:
			camera( camera ),
			material( pMaterial )
		{
			transform.world = world;
			transform.viewproj = viewproj;
		}
	public:
		CB_LightPipelineMatrix transform;
		const Camera& camera;
		Material& material;
	};

	class LightPipeline : public IPipeline
	{
	public:
		LightPipeline( const std::string& vsFilename, const std::string& psFilename );

		void BindParameters( IPipelineParameters& pParameters ) override;
		void Render( UINT vertexcount ) override;
		void RenderIndexed( UINT indexcount ) override;

		Light* GetLight() const { return m_pLight; }
		void SetLight( Light* pLight ) { m_pLight = pLight; }
	protected:
		Light* m_pLight = nullptr;
	};
}