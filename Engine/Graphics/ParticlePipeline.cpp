#include "PCH.h"
#include "ParticlePipeline.h"

#include "Camera.h"
#include "ShaderManager.h"

namespace Bat
{
	void ParticlePipeline::Render( IGPUContext* pContext,
		const std::vector<ParticleInstanceData>& instances,
		const ParticleEmitterComponent& emitter,
		const Camera& camera )
	{
		IVertexShader* pVertexShader = ResourceManager::GetVertexShader( "Graphics/Shaders/ParticleVS.hlsl" );
		IPixelShader* pPixelShader = ResourceManager::GetPixelShader( "Graphics/Shaders/ParticlePS.hlsl" );

		pContext->SetVertexShader( pVertexShader );
		pContext->SetPixelShader( pPixelShader );

		BindTransforms( pContext, camera );
		BindParticles( pContext, pVertexShader, instances, emitter );
		pContext->SetVertexBuffer( nullptr, 0 );
		pContext->BindTexture( emitter.texture->Get(), PS_TEX_SLOT_0 );

		pContext->Draw( instances.size() * 6 );
	}

	void ParticlePipeline::BindTransforms( IGPUContext* pContext, const Camera& camera )
	{
		CB_ParticlePipelineMatrix transform;
		transform.view = camera.GetViewMatrix();
		transform.proj = camera.GetProjectionMatrix();
		m_cbufTransform.Update( pContext, transform );
		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, VS_CBUF_TRANSFORMS );
	}

	void ParticlePipeline::BindParticles( IGPUContext* pContext,
		IVertexShader* pVertexShader,
		const std::vector<ParticleInstanceData>& instances,
		const ParticleEmitterComponent& emitter )
	{
		ASSERT( instances.size() <= MAX_PARTICLES, "Too many particles!" );
		
		auto buf = m_cbufParticles.Lock( pContext );
		memcpy( buf->particles, instances.data(), instances.size() * sizeof( ParticleInstanceData ) );
		buf->lifetime = emitter.lifetime;
		buf->start_alpha = emitter.start_alpha;
		buf->end_alpha = emitter.end_alpha;
		buf->start_scale = emitter.start_scale;
		buf->end_scale = emitter.end_scale;
		m_cbufParticles.Unlock( pContext );

		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufParticles, VS_CBUF_PARTICLES );
		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufParticles, PS_CBUF_PARTICLES );
	}
}