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
	class ForwardOpaquePass : public BaseRenderPass
	{
	public: // BaseRenderPass
		ForwardOpaquePass()
		{
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TARGET );
		}

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

			m_bGettingLights = true;
			Traverse( scene );
			m_bGettingLights = false;
			Traverse( scene );
		}
	private:
		virtual void Visit( const DirectX::XMMATRIX& transform, const SceneNode& node ) override
		{
			Entity e = node.Get();

			if( !m_bGettingLights )
			{
				if( e.Has<ModelComponent>() )
				{
					auto& model = e.Get<ModelComponent>();

					DirectX::XMMATRIX w = transform;
					DirectX::XMMATRIX vp = m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix();

					auto& meshes = model.GetMeshes();
					for( auto& pMesh : meshes )
					{
						// World space mins/maxs
						Vec3 wmins = DirectX::XMVector3Transform( pMesh->GetMins(), w );
						Vec3 wmaxs = DirectX::XMVector3Transform( pMesh->GetMaxs(), w );
						// Re-Aligned space mins/maxs
						// This just gets the AABB of the rotated AABB of the mesh
						// Will have lots of empty space, but better than recalculating AABB for rotated mesh vertices
						// TODO: Put a lot of this stuff in mathlib
						Vec3 awmins = { FLT_MAX, FLT_MAX, FLT_MAX };
						Vec3 awmaxs = { FLT_MIN, FLT_MIN, FLT_MIN };
						if( wmins.x < awmins.x )
						{
							awmins.x = wmins.x;
						}
						if( wmins.y < awmins.y )
						{
							awmins.y = wmins.y;
						}
						if( wmins.z < awmins.z )
						{
							awmins.z = wmins.z;
						}
						if( wmaxs.x < awmins.x )
						{
							awmins.x = wmaxs.x;
						}
						if( wmaxs.y < awmins.y )
						{
							awmins.y = wmaxs.y;
						}
						if( wmaxs.z < awmins.z )
						{
							awmins.z = wmaxs.z;
						}
						if( wmins.x > awmaxs.x )
						{
							awmaxs.x = wmins.x;
						}
						if( wmins.y > awmaxs.y )
						{
							awmaxs.y = wmins.y;
						}
						if( wmins.z > awmaxs.z )
						{
							awmaxs.z = wmins.z;
						}
						if( wmaxs.x > awmaxs.x )
						{
							awmaxs.x = wmaxs.x;
						}
						if( wmaxs.y > awmaxs.y )
						{
							awmaxs.y = wmaxs.y;
						}
						if( wmaxs.z > awmaxs.z )
						{
							awmaxs.z = wmaxs.z;
						}

						// View frustum culling on mesh level
						if( !m_pCamera->GetFrustum().IsBoxInside( awmins, awmaxs ) )
						{
							continue;
						}

						auto pPipeline = static_cast<LitGenericPipeline*>(ShaderManager::GetPipeline( "litgeneric" ));

						LitGenericPipelineParameters params( w, vp, pMesh->GetMaterial(), m_Lights, m_LightTransforms );
						pMesh->Bind( m_pContext, pPipeline );
						pPipeline->BindParameters( m_pContext, params );
						pPipeline->RenderIndexed( m_pContext, pMesh->GetIndexCount() );
					}
				}
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
	private:
		bool m_bGettingLights;
		Camera* m_pCamera = nullptr;
		std::vector<Entity> m_Lights;
		std::vector<DirectX::XMMATRIX> m_LightTransforms;
		IGPUContext* m_pContext = nullptr;
	};
}