#pragma once

#include "IRenderPass.h"
#include "Scene.h"
#include "RenderData.h"
#include "Graphics.h"
#include "Mesh.h"
#include "Material.h"
#include "ShaderManager.h"
#include "SceneLoader.h"
#include "LitGenericPipeline.h"

namespace Bat
{
	class DrawLightsPass : public BaseRenderPass, public ISceneVisitor
	{
	public: // BaseRenderPass
		DrawLightsPass()
		{
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TEXTURE );
		}

		virtual void Execute( IGPUContext* pContext, SceneGraph& scene, RenderData& data ) override
		{
			m_pCamera = scene.GetActiveCamera();

			IRenderTarget* target = data.GetRenderTarget( "dst" );
			pContext->SetRenderTarget( target );

			m_pContext = pContext;

			scene.AcceptVisitor( *this );
		}
	public: // ISceneVisitor
		virtual void Visit( const DirectX::XMMATRIX& transform, ISceneNode& node ) override
		{
			static Model* light_model = nullptr;
			static Material light_material;

			if( !light_model )
			{
				static std::unique_ptr<ISceneNode> light_model_node = SceneLoader::LoadScene( "Assets/sphere.gltf" );
				light_model = light_model_node->GetChildNodes()[2]->GetModel( 0 );
				light_model->SetScale( 5.0f );
			}

			m_pContext->SetDepthStencilEnabled( true );

			size_t count = node.GetLightCount();
			for( size_t i = 0; i < count; i++ )
			{
				Light* light = node.GetLight( i );
				if( light->GetType() != LightType::POINT )
				{
					continue;
				}

				auto emissive = light->GetColour() * 3;
				light_material.SetEmissiveColour( emissive.x, emissive.y, emissive.z );

				DirectX::XMMATRIX w = transform * light_model->GetWorldMatrix();
				DirectX::XMMATRIX vp = m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix();

				light_model->SetPosition( light->GetPosition() );
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
	private:
		Camera* m_pCamera = nullptr;
		IGPUContext* m_pContext = nullptr;
	};
}