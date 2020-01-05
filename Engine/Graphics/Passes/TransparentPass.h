#pragma once

#include "SceneRenderPass.h"
#include "Entity.h"
#include "CoreEntityComponents.h"
#include "Model.h"
#include "RenderData.h"
#include "Graphics.h"
#include "Mesh.h"
#include "Material.h"
#include "ShaderManager.h"
#include "LitGenericPipeline.h"
#include "ParticlePipeline.h"
#include "Particles.h"
#include "MeshBuilder.h"
#include <algorithm>

namespace Bat
{
	class TransparentPass : public SceneRenderPass
	{
	public: // BaseRenderPass
		TransparentPass()
		{
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TARGET );
		}
	private:
		virtual void PreRender( IGPUContext* pContext, Camera& camera, RenderData& data ) override
		{
			IRenderTarget* target = data.GetRenderTarget( "dst" );
			pContext->SetRenderTarget( target );

			pContext->SetDepthStencilEnabled( true );
			pContext->SetDepthWriteEnabled( false );
			pContext->SetBlendingEnabled( true );
			pContext->SetCullMode( CullMode::NONE );

			m_TranslucentMeshes.clear();
		}

		virtual void Render( const DirectX::XMMATRIX& transform, Entity e ) override
		{
			if( e.Has<ModelComponent>() )
			{
				const auto& model = e.Get<ModelComponent>();
				RenderModel( model, transform );
			}
			if( e.Has<ParticleEmitterComponent>() )
			{
				auto& emitter = e.Get<ParticleEmitterComponent>();
				RenderParticles( emitter, transform );
			}
		}

		void RenderModel( const ModelComponent& model, const DirectX::XMMATRIX& transform )
		{
			Camera* pCamera = GetCamera();

			DirectX::XMMATRIX w = transform;

			auto& meshes = model.GetMeshes();
			for( auto& pMesh : meshes )
			{
				if( !pMesh->GetMaterial().IsTranslucent() )
				{
					continue;
				}

				if( !MeshInCameraFrustum( pMesh.get(), pCamera, w ) )
				{
					continue;
				}

				TranslucentMesh translucent_mesh;
				translucent_mesh.mesh = pMesh.get();
				translucent_mesh.world_transform = w;
				m_TranslucentMeshes.push_back( translucent_mesh );
			}
		}

		void RenderParticles( ParticleEmitterComponent& emitter, const DirectX::XMMATRIX& transform )
		{
			IGPUContext* pContext = GetContext();
			Camera* pCamera = GetCamera();
			
			std::sort( emitter.particles.begin(), emitter.particles.begin() + emitter.num_particles, [pCamera]( const Particle& a, const Particle& b )
			{
				const Vec3& cam_pos = pCamera->GetPosition();

				const float a_distance_sq = (a.position - cam_pos).LengthSq();
				const float b_distance_sq = (b.position - cam_pos).LengthSq();

				return a_distance_sq > b_distance_sq;
			} );

			static std::vector<ParticleInstanceData> instances;
			instances.clear();
			instances.reserve( emitter.particles.size() );
			for( int i = 0; i < emitter.num_particles; i++ )
			{
				const Particle& particle = emitter.particles[i];
				ParticleInstanceData instance;
				instance.velocity = particle.velocity;
				instance.rot_velocity = particle.rot_velocity;
				instance.position = particle.position;
				instance.age = particle.age;
				instance.colour = particle.colour;
				instances.push_back( instance );
			}

			auto pPipeline = ShaderManager::GetPipeline<ParticlePipeline>();
			pPipeline->Render( pContext, instances, emitter, *pCamera );
		}

		virtual void PostRender( IGPUContext* pContext, Camera& camera, RenderData& data ) override
		{
			// Sort the meshes we got so that they are drawn back-to-front
			std::sort( m_TranslucentMeshes.begin(), m_TranslucentMeshes.end(), [camera]( const TranslucentMesh& a, const TranslucentMesh& b )
			{
				const Vec3& cam_pos = camera.GetPosition();
				const Vec3 a_pos = DirectX::XMVector3Transform( a.mesh->GetAABB().GetCenter(), a.world_transform );
				const Vec3 b_pos = DirectX::XMVector3Transform( b.mesh->GetAABB().GetCenter(), b.world_transform );

				const float a_distance_sq = (a_pos - cam_pos).LengthSq();
				const float b_distance_sq = (b_pos - cam_pos).LengthSq();
				
				return a_distance_sq > b_distance_sq;
			} );

			// Render the sorted meshes
			LightList light_list = SceneRenderPass::GetLights();
			auto pPipeline = ShaderManager::GetPipeline<LitGenericPipeline>();

			for( const TranslucentMesh& translucent_mesh : m_TranslucentMeshes )
			{
				pPipeline->Render( pContext, *translucent_mesh.mesh, camera, translucent_mesh.world_transform, light_list.entities, light_list.transforms );
			}
		}
	private:
		struct TranslucentMesh
		{
			Mesh* mesh;
			DirectX::XMMATRIX world_transform;
		};

		std::vector<TranslucentMesh> m_TranslucentMeshes;
	};
}