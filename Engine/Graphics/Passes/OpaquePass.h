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

			if( e.Has<ModelComponent>() )
			{
				auto& model = e.Get<ModelComponent>();

				DirectX::XMMATRIX w = transform;
				DirectX::XMMATRIX vp = pCamera->GetViewMatrix() * pCamera->GetProjectionMatrix();

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

					auto pPipeline = static_cast<LitGenericPipeline*>(ShaderManager::GetPipeline( "litgeneric" ));

					LitGenericPipelineParameters params( w, vp, pMesh->GetMaterial(), light_list.entities, light_list.transforms);
					pMesh->Bind( pContext, pPipeline );
					pPipeline->BindParameters( pContext, params );
					pPipeline->RenderIndexed( pContext, pMesh->GetIndexCount() );
				}
			}
		}
	};
}