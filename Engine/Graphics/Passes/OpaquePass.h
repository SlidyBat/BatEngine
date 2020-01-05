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
#include "DebugDraw.h"
#include "AnimationComponent.h"

namespace Bat
{
	class OpaquePass : public SceneRenderPass
	{
	public: // BaseRenderPass
		OpaquePass()
		{
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TARGET );
		}
	private:
		virtual void PreRender( IGPUContext* pContext, Camera& camera, RenderData& data ) override
		{
			IRenderTarget* target = data.GetRenderTarget( "dst" );
			pContext->SetRenderTarget( target );

			pContext->SetDepthStencilEnabled( true );
			pContext->SetDepthWriteEnabled( true );
			pContext->SetBlendingEnabled( false );
			pContext->SetCullMode( CullMode::BACK );
		}

		virtual void Render( const DirectX::XMMATRIX& transform, Entity e ) override
		{
			IGPUContext* pContext = SceneRenderPass::GetContext();
			Camera* pCamera = SceneRenderPass::GetCamera();
			LightList light_list = SceneRenderPass::GetLights();

			if( e.Has<AnimationComponent>() )
			{
				auto& anim = e.Get<AnimationComponent>();

				DirectX::XMMATRIX* cbuf = m_cbufBones.Lock( pContext )->bone_transforms;

				GetMatrixPalette( cbuf, anim.bones, anim.current_pose );

				m_cbufBones.Unlock( pContext );

				pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufBones, VS_CBUF_BONES );
			}
			if( e.Has<ModelComponent>() )
			{
				auto& model = e.Get<ModelComponent>();

				DirectX::XMMATRIX w = transform;

				if( model.HasRenderFlag( RenderFlags::DRAW_BBOX ) )
				{
					DrawOutlineBox( pContext, *pCamera, model.GetAABB(), w );
				}

				auto& meshes = model.GetMeshes();
				for( auto& pMesh : meshes )
				{
					if( pMesh->GetMaterial().IsTranslucent() )
					{
						continue;
					}

					if( !MeshInCameraFrustum( pMesh.get(), pCamera, w ) )
					{
						continue;
					}

					if( pMesh->HasRenderFlag( RenderFlags::INSTANCED ) )
					{
						LitGenericInstanceData data;
						data.world = w;

						auto& instances = m_mapInstancedMeshes[pMesh.get()];
						instances.push_back( data );

						continue;
					}

					if( pMesh->HasRenderFlag( RenderFlags::DRAW_BBOX ) )
					{
						DrawOutlineBox( pContext, *pCamera, pMesh->GetAABB(), w );
					}

					auto pPipeline = ShaderManager::GetPipeline<LitGenericPipeline>();
					pPipeline->Render( pContext, *pMesh, *pCamera, w, light_list.entities, light_list.transforms );
				}
			}
		}

		virtual void PostRender( IGPUContext* pContext, Camera& camera, RenderData& data ) override
		{
			LightList light_list = SceneRenderPass::GetLights();
			auto pPipeline = ShaderManager::GetPipeline<LitGenericPipeline>();
			
			for( auto& [pMesh, instances] : m_mapInstancedMeshes )
			{
				pPipeline->RenderInstanced( pContext, *pMesh, instances, camera, light_list.entities, light_list.transforms );
				instances.clear();
			}
		}

		void DrawOutlineBox( IGPUContext* pContext, const Camera& cam, const AABB& aabb, DirectX::XMMATRIX world_transform )
		{
			AABB transformed_aabb = aabb.Transform( world_transform );
			DebugDraw::Box( transformed_aabb.mins, transformed_aabb.maxs );
		}

		void GetMatrixPalette( DirectX::XMMATRIX* out, const std::vector<BoneData>& bones, SkeletonPose pose ) const
		{
			ASSERT( bones.size() <= MAX_BONES, "Too many bones!" );

			auto model_pose = SkeletonPose::ToModelSpace( std::move( pose ) );
			SkeletonPose::ToMatrixPalette( model_pose, bones, out );
		}
	private:
		struct CB_Bones
		{
			DirectX::XMMATRIX bone_transforms[MAX_BONES];
		};
		ConstantBuffer<CB_Bones> m_cbufBones;

		
		std::unordered_map<Mesh*, std::vector<LitGenericInstanceData>> m_mapInstancedMeshes;
	};
}