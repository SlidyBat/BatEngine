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
		virtual void PreRender( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data ) override
		{
			m_pCurrentAnimator = nullptr;

			IRenderTarget* target = data.GetRenderTarget( "dst" );
			pContext->SetRenderTarget( target );

			pContext->SetDepthStencilEnabled( true );
			pContext->SetDepthWriteEnabled( true );
			pContext->SetBlendingEnabled( false );
			pContext->SetCullMode( CullMode::BACK );
		}

		virtual void Render( const DirectX::XMMATRIX& transform, const SceneNode& node ) override
		{
			IGPUContext* pContext = SceneRenderPass::GetContext();
			Camera* pCamera = SceneRenderPass::GetCamera();
			LightList light_list = SceneRenderPass::GetLights();

			Entity e = node.Get();

			if( e.Has<AnimationComponent>() )
			{
				auto& anim = e.Get<AnimationComponent>();
				MeshAnimator* animator = anim.GetAnimator();
				if( animator != m_pCurrentAnimator )
				{
					m_pCurrentAnimator = animator;
					animator->Bind( pContext );
				}
			}
			if( e.Has<ModelComponent>() )
			{
				auto& name = e.Get<NameComponent>().name;
				auto& model = e.Get<ModelComponent>();

				DirectX::XMMATRIX w = transform;

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

					auto pPipeline = ShaderManager::GetPipeline<LitGenericPipeline>();
					pPipeline->Render( pContext, *pMesh, *pCamera, w, light_list.entities, light_list.transforms );
				}
			}
		}
	private:
		MeshAnimator* m_pCurrentAnimator = nullptr;
	};
}