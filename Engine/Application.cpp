#include "PCH.h"
#include "Application.h"

#include "Common.h"
#include "Graphics.h"
#include "Window.h"
#include "SceneLoader.h"
#include "FileWatchdog.h"

#include "WindowEvents.h"
#include "MouseEvents.h"
#include "NetworkEvents.h"
#include "TexturePipeline.h"
#include "Globals.h"
#include "IRenderPass.h"
#include "ShaderManager.h"
#include "RenderData.h"
#include "Passes/ClearRenderTargetPass.h"
#include "Passes/SkyboxPass.h"
#include "Passes/BloomPass.h"
#include "Passes/MotionBlurPass.h"
#include "Passes/ForwardOpaquePass.h"
#include "Passes/DrawLightsPass.h"

namespace Bat
{
	Application::Application( Graphics& gfx, Window& wnd )
		:
		gfx( gfx ),
		wnd( wnd ),
		scene( SceneLoader::LoadScene( "Assets/Ignore/Sponza/Sponza.gltf" ) )
	{
		Entity flashlight_ent = world.CreateEntity();
		flashlight = &world.AddComponent<LightComponent>( flashlight_ent ).light;
		flashlight->SetType( LightType::SPOT );
		flashlight->SetSpotlightAngle( 0.5f );
		scene.AddChildNode( flashlight_ent );

		Entity sun_ent = world.CreateEntity();
		sun = &world.AddComponent<LightComponent>( sun_ent ).light;
		sun->SetType( LightType::DIRECTIONAL );
		sun->SetEnabled( false );
		scene.AddChildNode( sun_ent );

		Entity cam_ent = world.CreateEntity();
		camera = new MoveableCamera( wnd.input, 250.0f, 100.0f );
		camera->SetPosition( { 0.0f, 0.0f, -10.0f } );
		world.AddComponent<CameraComponent>( cam_ent, camera );
		scene.AddChildNode( cam_ent );

		gfx.SetActiveScene( &scene );
		world.AddComponent<TransformComponent>( scene.Get(), DirectX::XMMatrixScaling( 0.5f, 0.5f, 0.5f ) );

		BuildRenderGraph();
		gfx.SetRenderGraph( &rendergraph );

		snd = Audio::CreateSoundPlaybackDevice();
		snd->SetListenerPosition( { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } );

		wnd.input.AddEventListener<KeyPressedEvent>( *this );

		/*g_Console.AddCommand( "load_scene", [&scene = scene, &cam = camera, &gfx = gfx]( const CommandArgs_t& args )
		{
			scene = SceneLoader::LoadScene( std::string( args[1] ) );
			scene.SetActiveCamera( &cam );
			gfx.SetActiveScene( &scene );
		} );

		g_Console.AddCommand( "add_model", [&scene = scene, &cam = camera]( const CommandArgs_t& args )
		{
			scene->AddChildNode( SceneLoader::LoadScene( std::string( args[1] ) ) );

			auto pos = cam.GetPosition();
			scene->GetChildNodes().back()->SetTransform( DirectX::XMMatrixTranslation( pos.x, pos.y, pos.z ) );
		} );*/

		g_Console.AddCommand( "cam_speed", [&cam = camera]( const CommandArgs_t& args )
		{
			float speed = std::stof( std::string( args[1] ) );
			cam->SetSpeed( speed );
		} );

		g_Console.AddCommand( "sun_toggle", [&sun = sun]( const CommandArgs_t & args )
		{
			sun->SetEnabled( !sun->IsEnabled() );
		} );
	}

	Application::~Application()
	{
		delete snd;
		delete camera;
	}

	void Application::OnUpdate( float deltatime )
	{
		elapsed_time += deltatime;
		fps_counter += 1;
		if( elapsed_time > 1.0f )
		{
			fps_string = std::to_string( fps_counter );
			fps_counter = 0;
			elapsed_time -= 1.0f;
		}

		camera->Update( deltatime );
		flashlight->SetPosition( camera->GetPosition() );
		flashlight->SetDirection( camera->GetLookAtVector() );
		snd->SetListenerPosition( camera->GetPosition(), camera->GetLookAtVector() );

		if( bloom_enabled )
		{
			auto bloom = static_cast<BloomPass*>( rendergraph.GetPassByName( "bloom" ) );
			if( bloom )
			{
				bloom->SetThreshold( bloom_threshold );
			}
		}
	}

	static void AddModelTree( Model& model )
	{
		if( ImGui::TreeNode( "Model" ) )
		{
			auto meshes = model.GetMeshes();
			for( auto mesh : meshes )
			{
				ImGui::Text( mesh->GetName().c_str() );
			}

			ImGui::TreePop();
		}
	}

	static void AddNodeTree( const SceneNode& node )
	{
		Entity e = node.Get();

		std::string name = "<blank>";
		if( world.HasComponent<NameComponent>( e ) )
		{
			name = world.GetComponent<NameComponent>( e ).name;
		}

		if( ImGui::TreeNode( name.c_str() ) )
		{
			size_t num_children = node.GetNumChildNodes();
			for( size_t i = 0; i < num_children; i++ )
			{
				AddNodeTree( node.GetChildNode( i ) );
			}

			if( world.HasComponent<ModelComponent>( e ) )
			{
				AddModelTree( world.GetComponent<ModelComponent>( e ).model );
			}

			ImGui::TreePop();
		}
	}

	void Application::OnRender()
	{
		if( imgui_menu_enabled )
		{
			ImGui::Text( "FPS: %s", fps_string.c_str() );

			ImGui::SliderFloat( "Bloom threshold", &bloom_threshold, 0.0f, 100.0f );

			AddNodeTree( scene );
		}
	}

	void Application::OnEvent( const KeyPressedEvent& e )
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
			// re-build render graph
			BuildRenderGraph();
		}
		else if( e.key == 'M' )
		{
			// toggle motion blur
			motion_blur_enabled = !motion_blur_enabled;
			// re-build render graoh
			BuildRenderGraph();
		}
		else if( e.key == 'C' )
		{
			Entity light_ent = world.CreateEntity();
			Light* light = &world.AddComponent<LightComponent>( light_ent ).light;
			light->SetPosition( camera->GetPosition() );
			light->SetRange( 250.0f );
			scene.AddChildNode( light_ent );
		}
		else if( e.key == 'F' )
		{
			flashlight->SetEnabled( !flashlight->IsEnabled() );
			snd->Play( "Assets/click.wav" );
		}
		else if( e.key == 'P' )
		{
			sun->SetDirection( camera->GetLookAtVector() );
		}
		else if( e.key == 'I' )
		{
			imgui_menu_enabled = !imgui_menu_enabled;
		}
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
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth() / 2, wnd.GetHeight() / 2, TEX_FORMAT_R32G32B32A32_FLOAT ) ) );
			// render texture 2 for bloom & motion blur
			rendergraph.AddRenderTextureResource( "rt2",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth() / 2, wnd.GetHeight() / 2, TEX_FORMAT_R32G32B32A32_FLOAT ) ) );
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

		rendergraph.AddPass( "forward_opaque", std::make_unique<ForwardOpaquePass>() );
		rendergraph.BindToResource( "forward_opaque.dst", "target" );

		rendergraph.AddPass( "draw_lights", std::make_unique<DrawLightsPass>() );
		rendergraph.BindToResource( "draw_lights.dst", "target" );

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
