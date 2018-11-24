#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <wrl.h>

#include "IPipeline.h"
#include "Texture.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "IModel.h"
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

	class LightModel : public IModel
	{
	public:
		LightModel( Mesh mesh );
		LightModel( std::vector<Mesh> meshes );

		virtual void Draw( IPipeline* pPipeline ) const override;
	private:
		std::vector<Mesh> m_Meshes;
	};

	class LightPipelineParameters : public IPipelineParameters
	{
	public:
		LightPipelineParameters( const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& viewproj, Material* pMaterial )
			:
			material( pMaterial )
		{
			transform.world = world;
			transform.viewproj = viewproj;
		}
	public:
		CB_LightPipelineMatrix transform;
		Material* material;
	};

	class LightPipeline : public IPipeline
	{
	public:
		LightPipeline( const std::wstring& vsFilename, const std::wstring& psFilename );

		void BindParameters( IPipelineParameters* pParameters ) override;
		void Render( UINT vertexcount ) override;
		void RenderIndexed( UINT indexcount ) override;

		Light* GetLight() const { return m_pLight; }
		void SetLight( Light* pLight ) { m_pLight = pLight; }
	private:
		VertexShader m_VertexShader;
		PixelShader m_PixelShader;

		Light* m_pLight = nullptr;
	};
}