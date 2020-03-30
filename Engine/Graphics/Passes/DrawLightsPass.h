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
		virtual void Visit( const Mat3x4& transform, Entity e ) override
		{
			if( e.Has<LightComponent>() )
			{
				static Resource<Mesh> sphere_mesh = ResourceManager::GetMesh( "Assets/sphere.gltf" );

				if( e.Has<LightComponent>() )
				{
					const auto& light = e.Get<LightComponent>();
					if( light.GetType() == LightType::POINT )
					{
						auto emissive = light.GetColour() * 3;

						sphere_mesh->GetMaterial().SetEmissiveFactor( emissive.x, emissive.y, emissive.z );

						const float scale = 0.05f;
						const auto w = Mat4( Mat3x4::Scale( scale ) * transform );

						auto pPipeline = ShaderManager::GetPipeline<LitGenericPipeline>();

						std::vector<Entity> empty;
						std::vector<Mat4> empty2;
						PbrGlobalMaps pbr_maps;
						pbr_maps.irradiance_map = nullptr;
						pbr_maps.prefilter_map = nullptr;
						pbr_maps.brdf_integration_map = nullptr;
						pPipeline->Render( m_pContext, *sphere_mesh, *m_pCamera, w, empty, empty2, pbr_maps );
					}
				}
			}
		}
	private:
		Camera* m_pCamera = nullptr;
		IGPUContext* m_pContext = nullptr;
	};
}