#include "PCH.h"
#include "Application.h"

#include "Common.h"
#include "Graphics.h"
#include "Window.h"
#include "SceneLoader.h"
#include "FileWatchdog.h"

#include "WindowEvents.h"
#include "KeyboardEvents.h"
#include "MouseEvents.h"
#include "NetworkEvents.h"
#include "TexturePipeline.h"
#include "LightPipeline.h"
#include "Globals.h"
#include "IRenderPass.h"
#include "ShaderManager.h"
#include "RenderData.h"
#include "Passes/ClearRenderTargetPass.h"
#include "Passes/SkyboxPass.h"

namespace Bat
{
	class SceneRenderer : public BaseRenderPass, public ISceneVisitor
	{
	public: // BaseRenderPass
		SceneRenderer( Graphics& gfx, Light* pLight = nullptr )
			:
			gfx( gfx ),
			m_pLight( pLight )
		{
			AddRenderNode( "dst", NodeType::OUTPUT, NodeDataType::RENDER_TEXTURE );
		}

		virtual void Execute( SceneGraph& scene, RenderData& data ) override
		{
			RenderTexture* target = data.GetRenderTexture( "dst" );
			target->Bind();

			scene.AcceptVisitor( *this );
		}
	public: // ISceneVisitor
		void SetLight( Light* pLight ) { m_pLight = pLight; }

		virtual void Visit( const DirectX::XMMATRIX& transform, ISceneNode& node ) override
		{
			RenderContext::SetDepthStencilEnabled( true );

			size_t count = node.GetModelCount();
			for( size_t i = 0; i < count; i++ )
			{
				Camera* cam = gfx.GetActiveScene()->GetActiveCamera();
				Model* pModel = node.GetModel( i );
				DirectX::XMMATRIX w = transform * pModel->GetWorldMatrix();
				DirectX::XMMATRIX vp = cam->GetViewMatrix() * cam->GetProjectionMatrix();

				pModel->Bind();

				auto& meshes = pModel->GetMeshes();
				for( auto& pMesh : meshes )
				{
					auto szPipelineName = pMesh->GetMaterial().GetDefaultPipelineName();
					auto pPipeline = static_cast<LightPipeline*>( ShaderManager::GetPipeline( szPipelineName ) );
					pPipeline->SetLight( m_pLight );

					Material material = pMesh->GetMaterial();
					LightPipelineParameters params( *cam, w, vp, material );
					pMesh->Bind( pPipeline );
					pPipeline->BindParameters( params );
					pPipeline->RenderIndexed( (UINT)pMesh->GetIndexCount() );
				}
			}
		}
	private:
		Graphics& gfx;
		Light* m_pLight = nullptr;
	};

	Application::Application( Graphics& gfx, Window& wnd )
		:
		gfx( gfx ),
		wnd( wnd ),
		camera( wnd.input ),
		scene( SceneLoader::LoadScene( "Assets\\Ignore\\Sponza\\Sponza.gltf" ) )
	{
		camera.SetPosition( { 0.0f, 0.0f, -10.0f } );

		light = scene.GetRootNode().AddLight( {} );
		scene.SetActiveCamera( &camera );
		gfx.SetActiveScene( &scene );

		InitializeRenderGraph();
		gfx.SetRenderGraph( &rendergraph );

		snd = Audio::CreateSoundPlaybackDevice();
		snd->SetListenerPosition( { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } );

		wnd.input.OnEventDispatched<KeyPressedEvent>( []( const KeyPressedEvent& e )
		{
			if( e.key == VK_OEM_3 )
			{
				BAT_LOG( "Toggling console" );
				g_Console.SetVisible( !g_Console.IsVisible() );
			}
		} );
	}

	Application::~Application()
	{
		delete snd;
	}

	void Application::OnUpdate( float deltatime )
	{
		elapsed_time += deltatime;
		fps_counter += 1;
		if( elapsed_time > 1.0f )
		{
			fps_string = "FPS: " + std::to_string( fps_counter );
			fps_counter = 0;
			elapsed_time -= 1.0f;
		}

		camera.Update( deltatime );
		snd->SetListenerPosition( camera.GetPosition(), camera.GetForwardVector() );
	}

	void Application::OnRender()
	{
		gfx.DrawText( Bat::StringToWide( fps_string ).c_str(), DirectX::XMFLOAT2{ 15.0f, 15.0f } );
	}

	void Application::InitializeRenderGraph()
	{
		// initialize resources
		rendergraph.AddRenderTextureResource( "backbuffer", std::make_unique<RenderTexture>( RenderTexture::Backbuffer() ) );
		rendergraph.AddTextureResource( "skybox", std::make_unique<Texture>( "Assets\\skybox.dds" ) );

		// add passes
		rendergraph.AddPass( "crt", std::make_unique<ClearRenderTargetPass>() );
		rendergraph.BindToResource( "crt.buffer", "backbuffer" );

		rendergraph.AddPass( "renderer", std::make_unique<SceneRenderer>( gfx, light ) );
		rendergraph.BindToResource( "renderer.dst", "backbuffer" );

		rendergraph.AddPass( "skybox", std::make_unique<SkyboxPass>() );
		rendergraph.BindToResource( "skybox.skyboxtex", "skybox" );
		rendergraph.MarkOutput( "skybox.dst" );
	}
}
