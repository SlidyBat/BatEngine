#pragma once

#include "IPipeline.h"
#include "ConstantBuffer.h"
#include "VertexBuffer.h"
#include "ShaderBuffers.h"
#include "Light.h"
#include "Material.h"

namespace Bat
{
	struct LitGenericInstanceData
	{
		DirectX::XMMATRIX world;
	};

	struct PbrGlobalMaps
	{
		ITexture* irradiance_map;
		ITexture* prefilter_map;
		ITexture* brdf_integration_map;
	};

	class LitGenericPipeline : public IPipeline
	{
	public:
		void Render( IGPUContext* pContext,
			const Mesh& mesh,
			const Camera& camera,
			const DirectX::XMMATRIX& world_transform,
			const std::vector<Entity>& light_ents,
			const std::vector<DirectX::XMMATRIX>& light_transforms,
			const PbrGlobalMaps& maps );

		void RenderInstanced( IGPUContext* pContext,
			const Mesh& mesh,
			const std::vector<LitGenericInstanceData>& instances,
			const Camera& camera,
			const std::vector<Entity>& light_ents,
			const std::vector<DirectX::XMMATRIX>& light_transforms,
			const PbrGlobalMaps& maps );
	private:
		void BindTransforms( IGPUContext* pContext,
			const Camera& camera,
			const DirectX::XMMATRIX& world_transform );
		void BindMaterial( IGPUContext* pContext, const Material& material );
		void BindLights( IGPUContext* pContext, const std::vector<Entity>& light_ents, const std::vector<DirectX::XMMATRIX>& light_transforms );
		void BindInstances( IGPUContext* pContext, IVertexShader* pVertexShader, const std::vector<LitGenericInstanceData>& instances );
	private:
		struct CB_LitGenericPipelineMatrix
		{
			DirectX::XMMATRIX world;
			DirectX::XMMATRIX viewproj;
		};

		struct CB_LitGenericPipelineMaterial
		{
			ShaderMaterial mat;
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
		VertexBuffer<LitGenericInstanceData> m_vbufInstanceData;
	};
}