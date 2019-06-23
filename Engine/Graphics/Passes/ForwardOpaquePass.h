#pragma once

#include "IRenderPass.h"
#include "Scene.h"
#include "RenderData.h"
#include "Graphics.h"
#include "Mesh.h"
#include "Material.h"
#include "ShaderManager.h"
#include "LitGenericPipeline.h"

namespace Bat
{
	class ForwardOpaquePass : public BaseRenderPass, public ISceneVisitor
	{
	public: // BaseRenderPass
		ForwardOpaquePass()
		{
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TEXTURE );
		}

		virtual void Execute( IGPUContext* pContext, SceneGraph& scene, RenderData& data ) override
		{
			m_Lights.clear();
			GetLights( scene.GetRootNode() );
			
			m_pCamera = scene.GetActiveCamera();

			IRenderTarget* target = data.GetRenderTarget( "dst" );
			pContext->SetRenderTarget( target );

			m_pContext = pContext;

			scene.AcceptVisitor( *this );
		}
	public: // ISceneVisitor
		virtual void Visit( const DirectX::XMMATRIX& transform, ISceneNode& node ) override
		{
			m_pContext->SetDepthStencilEnabled( true );

			size_t count = node.GetModelCount();
			for( size_t i = 0; i < count; i++ )
			{
				Model* pModel = node.GetModel( i );

				DirectX::XMMATRIX w = transform * pModel->GetWorldMatrix();
				DirectX::XMMATRIX vp = m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix();

				pModel->Bind();

				auto& meshes = pModel->GetMeshes();
				for( auto& pMesh : meshes )
				{
					// World space mins/maxs
					Vec3 wmins = DirectX::XMVector3Transform( pMesh->GetMins(), w );
					Vec3 wmaxs = DirectX::XMVector3Transform( pMesh->GetMaxs(), w );

					// View frustum culling on mesh level
					if( !m_pCamera->GetFrustum().IsBoxInside( wmins, wmaxs ) )
					{
						continue;
					}

					auto pPipeline = static_cast<LitGenericPipeline*>(ShaderManager::GetPipeline( "litgeneric" ));

					LitGenericPipelineParameters params( w, vp, pMesh->GetMaterial(), m_Lights );
					pMesh->Bind( m_pContext, pPipeline );
					pPipeline->BindParameters( m_pContext, params );
					pPipeline->RenderIndexed( m_pContext, pMesh->GetIndexCount() );
				}
			}
		}

		void GetLights( ISceneNode* node )
		{
			size_t num_lights = node->GetLightCount();
			for( size_t i = 0; i < num_lights; i++ )
			{
				Light* light = node->GetLight( i );

				// View frustum culling
				if( light->GetType() == LightType::POINT &&
					!m_pCamera->GetFrustum().IsSphereInside( light->GetPosition(), light->GetRange() ) )
				{
					continue;
				}

				m_Lights.push_back( light );
			}

			auto children = node->GetChildNodes();
			for( auto child : children )
			{
				GetLights( child );
			}
		}
	private:
		Camera* m_pCamera = nullptr;
		std::vector<Light*> m_Lights;
		IGPUContext* m_pContext = nullptr;
	};
}