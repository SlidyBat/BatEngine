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
		Vec3 velocity;
		float rot_velocity;
		Vec3 position;
		float age;
		Vec4 colour;
	};

	class ParticlePipeline : public IPipeline
	{
	public:
		void Render( IGPUContext* pContext,
			const std::vector<ParticleInstanceData>& instances,
			const ParticleEmitterComponent& emitter,
			const Camera& camera );
	private:
		void BindTransforms( IGPUContext* pContext, const Camera& camera );
		void BindParticles( IGPUContext* pContext,
			IVertexShader* pVertexShader,
			const std::vector<ParticleInstanceData>& instances,
			const ParticleEmitterComponent& emitter );
	private:
		struct CB_ParticlePipelineMatrix
		{
			DirectX::XMMATRIX view;
			DirectX::XMMATRIX proj;
		};
		ConstantBuffer<CB_ParticlePipelineMatrix> m_cbufTransform;

		struct CB_ParticlePipelineParticles
		{
			float lifetime;
			float start_alpha;
			float end_alpha;
			float start_scale;

			float end_scale;
			float motion_blur;
			float _pad0[2];

			ParticleInstanceData particles[MAX_PARTICLES];
		};
		ConstantBuffer<CB_ParticlePipelineParticles> m_cbufParticles;
	};
}