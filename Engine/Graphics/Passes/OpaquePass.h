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
					
					if( model.HasRenderFlag( RenderFlags::DRAW_BBOX ) )
					{
						DrawOutlineBox( pContext, *pCamera, pMesh->GetAABB(), w );
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

		void DrawOutlineBox( IGPUContext* pContext, const Camera& cam, const AABB& aabb, DirectX::XMMATRIX world_transform )
		{
			const Viewport& vp = pContext->GetViewport();

			DirectX::XMMATRIX wvp = cam.GetViewMatrix() * cam.GetProjectionMatrix();
			DirectX::XMMATRIX ndc_to_screen = DirectX::XMMatrixInverse( nullptr,
					DirectX::XMMatrixOrthographicOffCenterLH( 0.0f, vp.width, vp.height, 0.0f, cam.GetNear(), cam.GetFar() )
				);
			DirectX::XMMATRIX world_to_screen = wvp * ndc_to_screen;

			AABB transformed_aabb = aabb.Transform( world_transform );
			Vec3 v[8];
			transformed_aabb.GetPoints( v );

			int left = INT_MAX;
			int right = INT_MIN;
			int top = INT_MAX;
			int bottom = INT_MIN;

			for( int i = 0; i < 8; i++ )
			{
				Vec4 ndc = DirectX::XMVector4Transform( Vec4( v[i], 1.0f ), wvp );
				ndc /= ndc.w;

				if( ndc.z >= 1 || ndc.z <= 0 )
				{
					continue;
				}

				Vec3 screen_coords3d = DirectX::XMVector4Transform( ndc, ndc_to_screen );
				Vei2 screen_coords = { (int)screen_coords3d.x, (int)screen_coords3d.y };

				DebugDraw::Rectangle( screen_coords, screen_coords, Colours::Green );
				std::string debugstr = Bat::Format( "%.2f %.2f %.2f", v[i].x, v[i].y, v[i].z );
				DebugDraw::Text( debugstr, { screen_coords.x + 5, screen_coords.y + 50 } );
				
				if( screen_coords.x < left )   left = screen_coords.x;
				if( screen_coords.x > right )  right = screen_coords.x;
				if( screen_coords.y < top )    top = screen_coords.y;
				if( screen_coords.y > bottom ) bottom = screen_coords.y;
			}

			//DebugDraw::Rectangle( { left, top }, { right, bottom }, Colours::Red );
		}
	private:
		MeshAnimator* m_pCurrentAnimator = nullptr;
	};
}