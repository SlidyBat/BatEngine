#pragma once

#include "IPipeline.h"
#include "Colour.h"
#include "Mesh.h"
#include "ConstantBuffer.h"
#include "Particles.h"

namespace Bat
{
	struct ParticleInstanceData
	{
		Vec3 position;
		float _pad0;
	};

	class ParticlePipeline : public IPipeline
	{
	public:
		void Render( IGPUContext* pContext,
			const std::vector<ParticleInstanceData>& instances,
			ITexture* texture,
			const Camera& camera );
	private:
		void BindTransforms( IGPUContext* pContext, const Camera& camera );
		void BindInstances( IGPUContext* pContext, IVertexShader* pVertexShader, const std::vector<ParticleInstanceData>& instances );
	private:
		struct CB_ParticlePipelineMatrix
		{
			DirectX::XMMATRIX view;
			DirectX::XMMATRIX proj;
		};
		ConstantBuffer<CB_ParticlePipelineMatrix> m_cbufTransform;

		struct CB_ParticlePipelineParticles
		{
			ParticleInstanceData particles[MAX_PARTICLES];
		};
		ConstantBuffer<CB_ParticlePipelineParticles> m_cbufParticles;
	};
}