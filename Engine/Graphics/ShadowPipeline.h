#pragma once

#include "IPipeline.h"
#include "ConstantBuffer.h"
#include "VertexBuffer.h"
#include "ShaderBuffers.h"
#include "Light.h"
#include "Material.h"

namespace Bat
{
	class ShadowPipeline : public IPipeline
	{
	public:
		void Render( IGPUContext* pContext,
			const Mesh& mesh,
			const DirectX::XMMATRIX& viewproj,
			const DirectX::XMMATRIX& world_transform );
	private:
		void BindTransforms( IGPUContext* pContext, const DirectX::XMMATRIX& viewproj, const DirectX::XMMATRIX& world_transform );
		void BindMaterial( IGPUContext* pContext, const Material& material );
	private:
		struct CB_ShadowPipelineMatrix
		{
			DirectX::XMMATRIX world;
			DirectX::XMMATRIX viewproj;
		};
		struct CB_ShadowPipelineMaterial
		{
			Vec4 BaseColourFactor;
			int HasBaseColourTexture;
			float AlphaCutoff;
			float _pad0[2];
		};
	private:
		ConstantBuffer<CB_ShadowPipelineMatrix> m_cbufTransform;
		ConstantBuffer<CB_ShadowPipelineMaterial> m_cbufMaterial;
	};
}