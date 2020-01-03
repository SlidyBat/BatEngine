#include "PCH.h"
#include "ParticlePipeline.h"

#include "Camera.h"
#include "ShaderManager.h"

namespace Bat
{
	void ParticlePipeline::Render( IGPUContext* pContext,
		const std::vector<ParticleInstanceData>& instances,
		ITexture* texture,
		const Camera& camera )
	{
		IVertexShader* pVertexShader = ResourceManager::GetVertexShader( "Graphics/Shaders/ParticleVS.hlsl" );
		IPixelShader* pPixelShader = ResourceManager::GetPixelShader( "Graphics/Shaders/ParticlePS.hlsl" );

		pContext->SetVertexShader( pVertexShader );
		pContext->SetPixelShader( pPixelShader );

		BindTransforms( pContext, camera );
		BindInstances( pContext, pVertexShader, instances );
		pContext->SetVertexBuffer( nullptr, 0 );
		pContext->BindTexture( texture, PS_TEX_SLOT_0 );

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

	void ParticlePipeline::BindInstances( IGPUContext* pContext, IVertexShader* pVertexShader, const std::vector<ParticleInstanceData>& instances )
	{
		ASSERT( instances.size() <= MAX_PARTICLES, "Too many particles!" );
		
		auto buf = m_cbufParticles.Lock( pContext );
		memcpy( buf->particles, instances.data(), instances.size() * sizeof( ParticleInstanceData ) );
		m_cbufParticles.Unlock( pContext );

		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufParticles, VS_CBUF_PARTICLES );
		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufParticles, PS_CBUF_PARTICLES );
	}
}