#pragma once

#include "IPipeline.h"
#include "ConstantBuffer.h"
#include "ShaderBuffers.h"
#include "Light.h"
#include "Material.h"

namespace Bat
{
	struct LitGenericPipelineParameters : public IPipelineParameters
	{
		LitGenericPipelineParameters( DirectX::XMMATRIX world, DirectX::XMMATRIX viewproj, Material& material, const std::vector<Entity>& lights )
			:
			world( world ),
			viewproj( viewproj ),
			material( material ),
			lights( lights )
		{}

		DirectX::XMMATRIX world;
		DirectX::XMMATRIX viewproj;

		Material& material;
		const std::vector<Entity>& lights;
	};

	class LitGenericPipeline : public IPipeline
	{
	public:
		LitGenericPipeline( const std::string& vs_filename, const std::string& ps_filename );

		virtual void BindParameters( IGPUContext* pContext, IPipelineParameters& pParameters ) override;
		virtual void Render( IGPUContext* pContext, size_t vertexcount ) override;
		virtual void RenderIndexed( IGPUContext* pContext, size_t indexcount ) override;
	private:
		struct CB_LitGenericPipelineMatrix
		{
			DirectX::XMMATRIX world;
			DirectX::XMMATRIX viewproj;
		};

		struct CB_LitGenericPipelineMaterial
		{
			ShaderMaterial material;
		};

		struct CB_LitGenericPipelineLights
		{
			uint32_t    num_lights;
			float       _pad0[3];
			ShaderLight lights[MAX_LIGHTS];
		};
	private:
		ConstantBuffer<CB_LitGenericPipelineMatrix>   m_cbufTransform;
		ConstantBuffer<CB_LitGenericPipelineMaterial> m_cbufMaterial;
		ConstantBuffer<CB_LitGenericPipelineLights>    m_cbufLightParams;
	};
}