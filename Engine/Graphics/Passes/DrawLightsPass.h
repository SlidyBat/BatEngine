#pragma once

#include "IRenderPass.h"
#include "Entity.h"
#include "RenderData.h"
#include "Graphics.h"
#include "Mesh.h"
#include "Material.h"
#include "ShaderManager.h"
#include "SceneLoader.h"
#include "LitGenericPipeline.h"

namespace Bat
{
	class DrawLightsPass : public BaseRenderPass
	{
	public: // BaseRenderPass
		DrawLightsPass()
		{
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TARGET );
		}

		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data ) override
		{
			m_pCamera = &camera;

			IRenderTarget* target = data.GetRenderTarget( "dst" );
			pContext->SetRenderTarget( target );

			m_pContext = pContext;
			m_pContext->SetDepthStencilEnabled( true );
			pContext->SetBlendingEnabled( false );

			Traverse( scene );
		}
	public:
		virtual void Visit( const DirectX::XMMATRIX& transform, const SceneNode& node ) override
		{
			static Entity* light_model = nullptr;
			static Material light_material;

			if( !light_model )
			{
				static SceneNode light_model_node = SceneLoader::LoadScene( "Assets/sphere.gltf" );
				light_model = &light_model_node.GetChild( 2 ).Get();
				light_model->Get<TransformComponent>().SetScale( 0.05f );
			}

			Entity e = node.Get();
			if( e.Has<LightComponent>() )
			{
				auto light = e.Get<LightComponent>();
				if( light.GetType() == LightType::POINT )
				{
					auto emissive = light.GetColour() * 3;
					light_material.SetEmissiveColour( emissive.x, emissive.y, emissive.z );

					DirectX::XMMATRIX w = light_model->Get<TransformComponent>().GetTransform() * transform;
					DirectX::XMMATRIX vp = m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix();

					auto& meshes = light_model->Get<ModelComponent>().GetMeshes();
					for( auto& pMesh : meshes )
					{
						auto pPipeline = static_cast<LitGenericPipeline*>(ShaderManager::GetPipeline( "litgeneric" ));

						std::vector<Entity> empty;
						std::vector<DirectX::XMMATRIX> empty2;
						LitGenericPipelineParameters params( w, vp, light_material, empty, empty2 );
						pMesh->Bind( m_pContext, pPipeline );
						pPipeline->BindParameters( m_pContext, params );
						pPipeline->RenderIndexed( m_pContext, pMesh->GetIndexCount() );
					}
				}
			}
		}
	private:
		Camera* m_pCamera = nullptr;
		IGPUContext* m_pContext = nullptr;
	};
}