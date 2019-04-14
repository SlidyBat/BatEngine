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
#include "Passes/BloomPass.h"
#include "Passes/MotionBlurPass.h"

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

		virtual void Execute( IGPUContext* pContext, SceneGraph& scene, RenderData& data ) override
		{
			IRenderTarget* target = data.GetRenderTarget( "dst" );
			pContext->SetRenderTarget( target );

			m_pContext = pContext;

			scene.AcceptVisitor( *this );
		}
	public: // ISceneVisitor
		void SetLight( Light* pLight ) { m_pLight = pLight; }

		virtual void Visit( const DirectX::XMMATRIX& transform, ISceneNode& node ) override
		{
			m_pContext->SetDepthStencilEnabled( true );

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
					pMesh->Bind( m_pContext, pPipeline );
					pPipeline->BindParameters( m_pContext, params );
					pPipeline->RenderIndexed( m_pContext, pMesh->GetIndexCount() );
				}
			}
		}
	private:
		Graphics& gfx;
		Light* m_pLight = nullptr;
		IGPUContext* m_pContext = nullptr;
	};

	Application::Application( Graphics& gfx, Window& wnd )
		:
		gfx( gfx ),
		wnd( wnd ),
		camera( wnd.input, 100.0f ),
		scene( SceneLoader::LoadScene( "Assets\\Ignore\\Car\\Scene.gltf" ) )
	{
		camera.SetPosition( { 0.0f, 0.0f, -10.0f } );

		light = scene.GetRootNode().AddLight( {} );
		scene.SetActiveCamera( &camera );
		gfx.SetActiveScene( &scene );

		BuildRenderGraph();
		gfx.SetRenderGraph( &rendergraph );

		snd = Audio::CreateSoundPlaybackDevice();
		snd->SetListenerPosition( { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } );

		wnd.input.OnEventDispatched<KeyPressedEvent>( [&,light=light,&cam=camera]( const KeyPressedEvent& e )
		{
			if( e.key == VK_OEM_3 )
			{
				BAT_LOG( "Toggling console" );
				g_Console.SetVisible( !g_Console.IsVisible() );
			}
			else if( e.key == 'B' )
			{
				// toggle bloom
				bloom_enabled = !bloom_enabled;
				// re-build render graoh
				BuildRenderGraph();
			}
			else if( e.key == 'C' )
			{
				light->SetPosition( camera.GetPosition() );
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
	}

	void Application::BuildRenderGraph()
	{
		// start fresh
		rendergraph.Reset();

		int post_process_count = 0;
		if( bloom_enabled ) post_process_count++;
		if( motion_blur_enabled ) post_process_count++;

		// initialize resources
		// render texture to draw scene to
		if( post_process_count )
		{
			rendergraph.AddRenderTextureResource( "target",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT ) ) );

			// render texture 1 for bloom
			rendergraph.AddRenderTextureResource( "rt1",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT ) ) );
			// render texture 2 for bloom & motion blur
			rendergraph.AddRenderTextureResource( "rt2",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT ) ) );
		}
		else
		{
			rendergraph.AddRenderTextureResource( "target", nullptr );
		}

		auto depth = std::unique_ptr<IDepthStencil>( gpu->CreateDepthStencil( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R24G8_TYPELESS ) );
		gpu->GetContext()->SetDepthStencil( depth.get() );
		rendergraph.AddDepthStencilResource( "depth", std::move( depth ) );
		rendergraph.AddTextureResource( "skybox", std::unique_ptr<ITexture>( gpu->CreateTexture( "Assets\\skybox.dds" ) ) );

		// add passes
		rendergraph.AddPass( "crt", std::make_unique<ClearRenderTargetPass>() );
		rendergraph.BindToResource( "crt.buffer", "target" );
		rendergraph.BindToResource( "crt.depth", "depth" );

		rendergraph.AddPass( "renderer", std::make_unique<SceneRenderer>( gfx, light ) );
		rendergraph.BindToResource( "renderer.dst", "target" );

		rendergraph.AddPass( "skybox", std::make_unique<SkyboxPass>() );
		rendergraph.BindToResource( "skybox.skyboxtex", "skybox" );
		if( !post_process_count )
		{
			rendergraph.MarkOutput( "skybox.dst" );
		}
		else
		{
			rendergraph.BindToResource( "skybox.dst", "target" );

			if( bloom_enabled )
			{
				post_process_count--;

				rendergraph.AddPass( "bloom", std::make_unique<BloomPass>() );
				rendergraph.BindToResource( "bloom.src", "target" );
				rendergraph.BindToResource( "bloom.buffer1", "rt1" );
				rendergraph.BindToResource( "bloom.buffer2", "rt2" );
				if( !post_process_count )
				{
					rendergraph.MarkOutput( "bloom.dst" );
				}
				else
				{
					rendergraph.BindToResource( "bloom.dst", "rt2" );
				}
			}

			if( motion_blur_enabled )
			{
				post_process_count--;

				rendergraph.AddPass( "motionblur", std::make_unique<MotionBlurPass>() );
				rendergraph.BindToResource( "motionblur.src", bloom_enabled ? "rt2" : "target" );
				rendergraph.BindToResource( "motionblur.depth", "depth" );
				if( !post_process_count )
				{
					rendergraph.MarkOutput( "motionblur.dst" );
				}
				else
				{
					rendergraph.BindToResource( "motionblur.dst", "target" );
				}
			}
		}
	}
}
