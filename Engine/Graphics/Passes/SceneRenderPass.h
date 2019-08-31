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
			LightList( const std::vector<Entity>& lights, const std::vector<DirectX::XMMATRIX> light_transforms )
				:
				entities( lights ),
				transforms( light_transforms )
			{}

			const std::vector<Entity>& entities;
			const std::vector<DirectX::XMMATRIX> transforms;
		};
		LightList GetLights() const
		{
			return LightList( m_Lights, m_LightTransforms );
		}

		// Helper function that checks if a mesh is contained inside the view frustum
		// The transform matrix passed in gets applied to mesh before doing check
		bool MeshInCameraFrustum( Mesh* pMesh, Camera* pCamera, DirectX::XMMATRIX transform )
		{
			// World space mins/maxs
			Vec3 transformed_mins = DirectX::XMVector3Transform( pMesh->GetMins(), transform );
			Vec3 transformed_maxs = DirectX::XMVector3Transform( pMesh->GetMaxs(), transform );
			// Re-Aligned space mins/maxs
			// This just gets the AABB of the rotated AABB of the mesh
			// Will have lots of empty space, but better than recalculating AABB for rotated mesh vertices
			// TODO: Put a lot of this stuff in mathlib
			Vec3 aligned_mins = { FLT_MAX, FLT_MAX, FLT_MAX };
			Vec3 aligned_maxs = { FLT_MIN, FLT_MIN, FLT_MIN };
			if( transformed_mins.x < aligned_mins.x ) aligned_mins.x = transformed_mins.x;
			if( transformed_mins.y < aligned_mins.y ) aligned_mins.y = transformed_mins.y;
			if( transformed_mins.z < aligned_mins.z ) aligned_mins.z = transformed_mins.z;
			if( transformed_maxs.x < aligned_mins.x ) aligned_mins.x = transformed_maxs.x;
			if( transformed_maxs.y < aligned_mins.y ) aligned_mins.y = transformed_maxs.y;
			if( transformed_maxs.z < aligned_mins.z ) aligned_mins.z = transformed_maxs.z;
			if( transformed_mins.x > aligned_maxs.x ) aligned_maxs.x = transformed_mins.x;
			if( transformed_mins.y > aligned_maxs.y ) aligned_maxs.y = transformed_mins.y;
			if( transformed_mins.z > aligned_maxs.z ) aligned_maxs.z = transformed_mins.z;
			if( transformed_maxs.x > aligned_maxs.x ) aligned_maxs.x = transformed_maxs.x;
			if( transformed_maxs.y > aligned_maxs.y ) aligned_maxs.y = transformed_maxs.y;
			if( transformed_maxs.z > aligned_maxs.z ) aligned_maxs.z = transformed_maxs.z;

			// View frustum culling on mesh level
			if( !pCamera->GetFrustum().IsBoxInside( aligned_mins, aligned_maxs ) )
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