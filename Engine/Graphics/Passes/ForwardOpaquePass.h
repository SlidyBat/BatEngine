#pragma once

#include "IRenderPass.h"
#include "Entity.h"
#include "CoreEntityComponents.h"
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
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TEXTURE );
		}

		virtual void Execute( IGPUContext* pContext, SceneNode& scene, RenderData& data ) override
		{
			m_pCamera = FindCamera( scene );
			if( !m_pCamera )
			{
				return;
			}

			m_Lights.clear();
			GetLights( scene );

			IRenderTarget* target = data.GetRenderTarget( "dst" );
			pContext->SetRenderTarget( target );

			m_pContext = pContext;
			m_pContext->SetDepthStencilEnabled( true );

			DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();
			Entity e = scene.Get();
			if( world.HasComponent<TransformComponent>( e ) )
			{
				transform = world.GetComponent<TransformComponent>( e ).transform;
			}

			Traverse( scene );
		}
	private:
		void Traverse( const SceneNode& scene )
		{
			std::stack<const SceneNode*> stack;
			std::stack<DirectX::XMMATRIX> transforms;

			stack.push( &scene );

			DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();
			if( world.HasComponent<TransformComponent>( scene.Get() ) )
			{
				transform = world.GetComponent<TransformComponent>( scene.Get() ).transform;
			}
			transforms.push( transform );

			while( !stack.empty() )
			{
				const SceneNode* node = stack.top();
				stack.pop();
				transform = transforms.top();
				transforms.pop();

				Entity e = node->Get();

				Draw( transform, *node );

				size_t num_children = node->GetNumChildNodes();
				for( size_t i = 0; i < num_children; i++ )
				{
					stack.push( &node->GetChildNode( i ) );

					if( world.HasComponent<TransformComponent>( node->Get() ) )
					{
						transforms.push( world.GetComponent<TransformComponent>( node->Get() ).transform * transform );
					}
					else
					{
						transforms.push( transform );
					}
				}
			}
		}

		void Draw( const DirectX::XMMATRIX& transform, const SceneNode& node )
		{
			Entity e = node.Get();

			if( world.HasComponent<ModelComponent>( e ) )
			{
				Model& model = world.GetComponent<ModelComponent>( e ).model;

				DirectX::XMMATRIX w = transform * model.GetWorldMatrix();
				DirectX::XMMATRIX vp = m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix();

				model.Bind();

				auto& meshes = model.GetMeshes();
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

		void GetLights( SceneNode& node )
		{
			Entity e = node.Get();
			if( world.HasComponent<LightComponent>( e ) )
			{
				Light& light = world.GetComponent<LightComponent>( e ).light;

				// View frustum culling
				if( light.GetType() != LightType::POINT ||
					m_pCamera->GetFrustum().IsSphereInside( light.GetPosition(), light.GetRange() ) )
				{
					m_Lights.push_back( &light );
				}
			}

			size_t num_children = node.GetNumChildNodes();
			for( size_t i = 0; i < num_children; i++ )
			{
				GetLights( node.GetChildNode( i ) );
			}
		}
	private:
		Camera* m_pCamera = nullptr;
		std::vector<Light*> m_Lights;
		IGPUContext* m_pContext = nullptr;
	};
}