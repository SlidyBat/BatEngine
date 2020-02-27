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
			m_pContext->SetDepthEnabled( true );
			pContext->SetBlendingEnabled( false );

			TraverseLights();

			PreRender( pContext, camera, data );
			Traverse();
			PostRender( pContext, camera, data );
		}
	private:
		void TraverseLights()
		{
			for( Entity e : world )
			{
				if( e.Has<LightComponent>() )
				{
					auto& hier = e.Get<HierarchyComponent>();
					m_Lights.push_back( e );
					m_LightTransforms.push_back( hier.GetAbsTransform() );
				}
			}
		}

		virtual void Visit( const Mat4& transform, Entity e ) override
		{
			Render( transform, e );
		}
	protected:
		virtual void PreRender( IGPUContext* pContext, Camera& camera, RenderData& data ) {};
		virtual void Render( const Mat4& transform, Entity e ) = 0;
		virtual void PostRender( IGPUContext* pContext, Camera& camera, RenderData& data ) {};

		IGPUContext* GetContext() { return m_pContext; }
		Camera* GetCamera() { return m_pCamera; }

		struct LightList
		{
			LightList( const std::vector<Entity>& lights, const std::vector<Mat4>& light_transforms )
				:
				entities( lights ),
				transforms( light_transforms )
			{}

			const std::vector<Entity>& entities;
			const std::vector<Mat4>& transforms;
		};
		LightList GetLights() const
		{
			return LightList( m_Lights, m_LightTransforms );
		}

		// Helper function that checks if a mesh is contained inside the view frustum
		// The transform matrix passed in gets applied to mesh before doing check
		bool MeshInCameraFrustum( Mesh* pMesh, Camera* pCamera, const Mat4& transform )
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
		Camera* m_pCamera = nullptr;
		std::vector<Entity> m_Lights;
		std::vector<Mat4> m_LightTransforms;
		IGPUContext* m_pContext = nullptr;
	};
}