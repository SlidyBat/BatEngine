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
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TEXTURE );
		}

		virtual void Execute( IGPUContext* pContext, SceneNode& scene, RenderData& data ) override
		{
			m_pCamera = FindCamera( scene );

			IRenderTarget* target = data.GetRenderTarget( "dst" );
			pContext->SetRenderTarget( target );

			m_pContext = pContext;
			m_pContext->SetDepthStencilEnabled( true );

			Traverse( scene );
		}
	public:
		void Traverse( const SceneNode& scene )
		{
			std::stack<const SceneNode*> stack;
			std::stack<DirectX::XMMATRIX> transforms;

			stack.push( &scene );

			DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();
			if( scene.Get().Has<TransformComponent>() )
			{
				transform = scene.Get().Get<TransformComponent>().GetTransform();
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

					if( node->Get().Has<TransformComponent>() )
					{
						transforms.push( node->Get().Get<TransformComponent>().GetTransform() * transform );
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
			static Model* light_model = nullptr;
			static Material light_material;

			if( !light_model )
			{
				static SceneNode light_model_node = SceneLoader::LoadScene( "Assets/sphere.gltf" );
				Entity light_model_ent = light_model_node.GetChildNode( 2 ).Get();
				light_model = &light_model_ent.Get<ModelComponent>().model;
				light_model->SetScale( 5.0f );
			}

			Entity e = node.Get();
			if( e.Has<LightComponent>() )
			{
				auto light = e.Get<LightComponent>();
				if( light.GetType() == LightType::POINT )
				{
					auto emissive = light.GetColour() * 3;
					light_material.SetEmissiveColour( emissive.x, emissive.y, emissive.z );

					DirectX::XMMATRIX w = transform * light_model->GetWorldMatrix();
					DirectX::XMMATRIX vp = m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix();

					light_model->SetPosition( e.Get<TransformComponent>().GetPosition() );
					light_model->Bind();

					auto& meshes = light_model->GetMeshes();
					for( auto& pMesh : meshes )
					{
						auto pPipeline = static_cast<LitGenericPipeline*>(ShaderManager::GetPipeline( "litgeneric" ));

						LitGenericPipelineParameters params( w, vp, light_material, {} );
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