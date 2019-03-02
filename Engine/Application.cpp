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
#include "RenderData.h"

namespace Bat
{
	class ClearRenderTargetPass : public BaseRenderPass
	{
	public:
		ClearRenderTargetPass()
		{
			AddRenderNode( "buffer", NodeType::INPUT, NodeDataType::RENDER_TEXTURE );
		}

		virtual std::string GetDescription() const override { return "Clears render target"; }

		virtual void Execute( SceneGraph& scene, RenderData& data ) override
		{
			RenderTexture* target = data.GetRenderTexture( "buffer" );
			target->Clear( 0.0f, 0.0f, 0.0f, 1.0f );
		}
	};

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
			size_t count = node.GetModelCount();
			for( size_t i = 0; i < count; i++ )
			{
				Model* pModel = node.GetModel( i );
				DirectX::XMMATRIX w = transform * pModel->GetWorldMatrix();
				DirectX::XMMATRIX vp = gfx.GetActiveCamera()->GetViewMatrix() * gfx.GetActiveCamera()->GetProjectionMatrix();

				pModel->Bind();

				auto& meshes = pModel->GetMeshes();
				for( auto& pMesh : meshes )
				{
					auto szPipelineName = pMesh->GetMaterial().GetDefaultPipelineName();
					auto pPipeline = static_cast<LightPipeline*>( gfx.GetPipeline( szPipelineName ) );
					pPipeline->SetLight( m_pLight );

					Material material = pMesh->GetMaterial();
					LightPipelineParameters params( *gfx.GetActiveCamera(), w, vp, material );
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
		scene( SceneLoader::LoadScene( "Assets\\light.fbx" ) )
	{
		// yuck! need to clean up scene graph usage in the future
		ISceneNode* node = &scene.GetRootNode();
		while( !node->GetChildNodes().empty() )
		{
			node = node->GetChildNodes()[0];
		}
		model = node->GetModel( 0 );

		light = scene.GetRootNode().AddLight( {} );
		gfx.SetActiveScene( &scene );
		gfx.SetActiveCamera( &camera );

		rendergraph.AddRenderTextureResource( "backbuffer", std::make_unique<RenderTexture>( RenderTexture::Backbuffer() ) );
		rendergraph.AddPass( "crt", std::make_unique<ClearRenderTargetPass>() );
		rendergraph.AddPass( "renderer", std::make_unique<SceneRenderer>( gfx, light ) );

		rendergraph.BindToResource( "crt.buffer", "backbuffer" );
		rendergraph.MarkOutput( "renderer.dst" );

		gfx.SetRenderGraph( &rendergraph );

		snd = Audio::CreateSoundPlaybackDevice();
		snd->SetListenerPosition( { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } );
		bell = snd->Play3DEx( "Assets\\bell.wav", { 10.0f, 10.0f, 0.0f }, SOUND_LOOP );

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
		snd->SetListenerPosition( camera.GetPosition(), camera.GetRotation() );

		bell->SetWorldPosition( { 5 * sin( g_pGlobals->elapsed_time ), 0.0f, 5 * cos( g_pGlobals->elapsed_time ) } );
		model->SetPosition( bell->GetWorldPosition() );
	}

	void Application::OnRender()
	{
		gfx.DrawText( Bat::StringToWide( fps_string ).c_str(), DirectX::XMFLOAT2{ 15.0f, 15.0f } );
	}
}
