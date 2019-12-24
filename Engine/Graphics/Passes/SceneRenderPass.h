#pragma once

#include "IRenderPass.h"
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
	class SceneRenderPass : public BaseRenderPass
	{
	public:
		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data ) override
		{
			m_pCamera = &camera;
			if( !m_pCamera )
			{
				return;
			}

			m_Lights.clear();
			m_LightTransforms.clear();

			IRenderTarget* target = data.GetRenderTarget( "dst" );
			pContext->SetRenderTarget( target );

			m_pContext = pContext;
			m_pContext->SetDepthStencilEnabled( true );
			pContext->SetBlendingEnabled( false );

			m_bGettingLights = true;
			Traverse( scene );
			m_bGettingLights = false;

			PreRender( pContext, camera, scene, data );
			Traverse( scene );
			PostRender( pContext, camera, scene, data );
		}
	private:
		virtual void Visit( const DirectX::XMMATRIX& transform, const SceneNode& node ) override
		{
			Entity e = node.Get();

			if( !m_bGettingLights )
			{
				Render( transform, node );
			}
			else
			{
				if( e.Has<LightComponent>() )
				{
					auto& light = e.Get<LightComponent>();

					if( light.GetType() == LightType::POINT )
					{
						// View frustum culling
						DirectX::XMVECTOR vs, vr, vp;
						DirectX::XMMatrixDecompose( &vs, &vr, &vp, transform );

						if( !m_pCamera->GetFrustum().IsSphereInside( vp, light.GetRange() ) )
						{
							return;
						}
					}

					m_Lights.push_back( e );
					m_LightTransforms.push_back( transform );
				}
			}
		}
	protected:
		virtual void PreRender( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data ) {};
		virtual void Render( const DirectX::XMMATRIX& transform, const SceneNode& node ) = 0;
		virtual void PostRender( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data ) {};

		IGPUContext* GetContext() { return m_pContext; }
		Camera* GetCamera() { return m_pCamera; }

		struct LightList
		{
			LightList( const std::vector<Entity>& lights, const std::vector<DirectX::XMMATRIX>& light_transforms )
				:
				entities( lights ),
				transforms( light_transforms )
			{}

			const std::vector<Entity>& entities;
			const std::vector<DirectX::XMMATRIX>& transforms;
		};
		LightList GetLights() const
		{
			return LightList( m_Lights, m_LightTransforms );
		}

		// Helper function that checks if a mesh is contained inside the view frustum
		// The transform matrix passed in gets applied to mesh before doing check
		bool MeshInCameraFrustum( Mesh* pMesh, Camera* pCamera, DirectX::XMMATRIX transform )
		{
			const AABB& aabb = pMesh->GetAABB();
			AABB world_aabb = aabb.Transform( transform );

			// View frustum culling on mesh level
			if( !pCamera->GetFrustum().IsBoxInside( world_aabb.mins, world_aabb.maxs ) )
			{
				return false;
			}

			return true;
		}
	private:
		bool m_bGettingLights;
		Camera* m_pCamera = nullptr;
		std::vector<Entity> m_Lights;
		std::vector<DirectX::XMMATRIX> m_LightTransforms;
		IGPUContext* m_pContext = nullptr;
	};
}