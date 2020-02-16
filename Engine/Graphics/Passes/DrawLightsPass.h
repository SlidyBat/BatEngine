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
			m_pContext->SetDepthEnabled( true );
			m_pContext->SetDepthWriteEnabled( true );
			pContext->SetBlendingEnabled( false );
			pContext->SetCullMode( CullMode::BACK );

			Traverse();
		}
	public:
		virtual void Visit( const DirectX::XMMATRIX& transform, Entity e ) override
		{
			static Entity* light_model = nullptr;
			static Material light_material;

			if( !light_model )
			{
				static SceneNode light_model_node = SceneLoader().Load( "Assets/sphere.gltf" );
				light_model = &light_model_node.GetChild( 2 ).Get();
				light_model->Get<TransformComponent>().SetScale( 0.05f );
			}

			if( e.Has<LightComponent>() )
			{
				const auto& light = e.Get<LightComponent>();
				if( light.GetType() == LightType::POINT )
				{
					auto emissive = light.GetColour() * 3;
					
					DirectX::XMMATRIX w = light_model->Get<TransformComponent>().GetTransform() * transform;

					auto& meshes = light_model->Get<ModelComponent>().GetMeshes();
					for( auto& pMesh : meshes )
					{
						pMesh->GetMaterial().SetEmissiveFactor( emissive.x, emissive.y, emissive.z );

						auto pPipeline = ShaderManager::GetPipeline<LitGenericPipeline>();

						std::vector<Entity> empty;
						std::vector<DirectX::XMMATRIX> empty2;
						PbrGlobalMaps pbr_maps;
						pbr_maps.irradiance_map = nullptr;
						pbr_maps.prefilter_map = nullptr;
						pbr_maps.brdf_integration_map = nullptr;
						pPipeline->Render( m_pContext, *pMesh, *m_pCamera, w, empty, empty2, pbr_maps );
					}
				}
			}
		}
	private:
		Camera* m_pCamera = nullptr;
		IGPUContext* m_pContext = nullptr;
	};
}