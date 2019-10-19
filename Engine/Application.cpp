#include "PCH.h"
#include "Application.h"

#include "Common.h"
#include "Graphics.h"
#include "Window.h"
#include "SceneLoader.h"
#include "FileWatchdog.h"

#include "CoreEntityComponents.h"
#include "WindowEvents.h"
#include "MouseEvents.h"
#include "NetworkEvents.h"
#include "PhysicsEvents.h"
#include "TexturePipeline.h"
#include "Globals.h"
#include "IRenderPass.h"
#include "ShaderManager.h"
#include "RenderData.h"
#include "RenderTarget.h"
#include "Passes/ClearRenderTargetPass.h"
#include "Passes/SkyboxPass.h"
#include "Passes/BloomPass.h"
#include "Passes/MotionBlurPass.h"
#include "Passes/OpaquePass.h"
#include "Passes/TransparentPass.h"
#include "Passes/DrawLightsPass.h"

namespace Bat
{
	Application::Application( Graphics& gfx, Window& wnd )
		:
		gfx( gfx ),
		wnd( wnd ),
		camera( wnd.input, 2.0f, 100.0f )
	{
		camera.SetAspectRatio( (float)wnd.GetWidth() / wnd.GetHeight() );

		scene.Set( world.CreateEntity() ); // Root entity;
		size_t scene_index = scene.AddChild( SceneLoader::LoadScene( "Assets/Ignore/Sponza/Sponza.gltf" ) );

		flashlight = world.CreateEntity();
		flashlight.Add<LightComponent>()
			.SetType( LightType::SPOT )
			.SetSpotlightAngle( 0.5f );
		flashlight.Add<TransformComponent>();
		scene.AddChild( flashlight );

		sun = world.CreateEntity();
		sun.Add<LightComponent>()	
			.SetType( LightType::DIRECTIONAL )
			.SetEnabled( false );
		scene.AddChild( sun );

		gfx.SetActiveScene( &scene );
		gfx.SetActiveCamera( &camera );

		BuildRenderGraph();
		gfx.SetRenderGraph( &rendergraph );

		// Initialize sound device
		snd = Audio::CreateSoundPlaybackDevice();
		snd->SetListenerPosition( { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } );

		// Initialize physics objects
		{
			Physics::EnableFixedTimestep( 1.0f / 60.0f );
			auto trigger = Physics::CreateStaticObject( { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } );
			trigger->AddBoxTrigger( 5.0f, 5.0f, 5.0f );
			player = Physics::CreateDynamicObject( camera.GetPosition(), camera.GetRotation() );
			player->AddSphereShape( 0.1f );
			player->SetKinematic( true );

			Entity scene_ent = scene.GetChild( scene_index ).Get();
			TransformComponent& scene_transform = scene_ent.Add<TransformComponent>().SetScale( 0.01f );
			ModelComponent& scene_model = scene_ent.Get<ModelComponent>();
			IStaticObject* scene_phys = Physics::CreateStaticObject( scene_transform.GetPosition(), scene_transform.GetRotation() );
			for( const auto& mesh : scene_model.GetMeshes() )
			{
				scene_phys->AddMeshShape( mesh->GetVertexData(), mesh->GetVertexCount(), mesh->GetIndexData(), mesh->GetIndexCount(), scene_transform.GetScale() );
			}
		}

		wnd.input.AddEventListener<KeyPressedEvent>( *this );

		/*g_Console.AddCommand( "load_scene", [&scene = scene, &cam = camera, &gfx = gfx]( const CommandArgs_t& args )
		{
			scene = SceneLoader::LoadScene( std::string( args[1] ) );
			scene.SetActiveCamera( &cam );
			gfx.SetActiveScene( &scene );
		} );

		g_Console.AddCommand( "add_model", [&scene = scene, &cam = camera]( const CommandArgs_t& args )
		{
			scene->AddChild( SceneLoader::LoadScene( std::string( args[1] ) ) );

			auto pos = cam.GetPosition();
			scene->GetChildNodes().back()->SetTransform( DirectX::XMMatrixTranslation( pos.x, pos.y, pos.z ) );
		} );*/

		g_Console.AddCommand( "cam_speed", [&cam = camera]( const CommandArgs_t& args )
		{
			float speed = std::stof( std::string( args[1] ) );
			cam.SetSpeed( speed );
		} );

		g_Console.AddCommand( "sun_toggle", [&sun = sun]( const CommandArgs_t & args )
		{
			LightComponent& l = sun.Get<LightComponent>();
			l.SetEnabled( !l.IsEnabled() );
		} );

		EventDispatcher::OnGlobalEventDispatched<PhysicsTriggerStartTouchEvent>( []( const PhysicsTriggerStartTouchEvent & e )
		{
			BAT_LOG( "Entered trigger" );
		} );
		EventDispatcher::OnGlobalEventDispatched<PhysicsTriggerEndTouchEvent>( []( const PhysicsTriggerEndTouchEvent & e )
		{
			BAT_LOG( "Exited trigger" );
		} );
		EventDispatcher::OnGlobalEventDispatched<PhysicsObjectStartTouchEvent>( []( const PhysicsObjectStartTouchEvent & e )
		{
			BAT_LOG( "Touch!" );
		} );

		wnd.AddEventListener<WindowResizeEvent>( *this );
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
			fps_string = std::to_string( fps_counter );
			fps_counter = 0;
			elapsed_time -= 1.0f;
		}

		camera.Update( deltatime );
		flashlight.Get<TransformComponent>().SetPosition( camera.GetPosition() );
		flashlight.Get<LightComponent>().SetDirection( camera.GetLookAtVector() );
		snd->SetListenerPosition( camera.GetPosition(), camera.GetLookAtVector() );

		player->MoveTo( camera.GetPosition(), camera.GetRotation() );

		// Sync light entity transform with physics object transform
		// Only manually for now
		for( size_t i = 0; i < lights.size(); i++ )
		{
			auto& transform = lights[i].Get<TransformComponent>();
			transform.SetPosition( lights_phys[i]->GetPosition() );
			transform.SetRotation( lights_phys[i]->GetRotation() );
		}

		if( bloom_enabled )
		{
			auto bloom = static_cast<BloomPass*>( rendergraph.GetPassByName( "bloom" ) );
			if( bloom )
			{
				bloom->SetThreshold( bloom_threshold );
			}
		}

		Physics::Simulate( deltatime );
	}

	static void AddModelTree( const ModelComponent& model )
	{
		if( ImGui::TreeNode( "Model" ) )
		{
			auto meshes = model.GetMeshes();
			for( const auto& mesh : meshes )
			{
				ImGui::Text( mesh->GetName().c_str() );
			}

			ImGui::TreePop();
		}
	}

	static void AddNodeTree( const SceneNode& node )
	{
		Entity e = node.Get();

		std::string name;
		if( world.HasComponent<NameComponent>( e ) )
		{
			name = e.Get<NameComponent>().name;
		}
		else
		{
			name = Format( "ent_%i", e.GetId().GetIndex() );
		}

		if( ImGui::TreeNode( name.c_str() ) )
		{
			size_t num_children = node.GetNumChildren();
			for( size_t i = 0; i < num_children; i++ )
			{
				AddNodeTree( node.GetChild( i ) );
			}

			if( e.Has<ModelComponent>() )
			{
				AddModelTree( e.Get<ModelComponent>() );
			}
			if( e.Has<LightComponent>() )
			{
				ImGui::Text( "Light" );
			}

			ImGui::TreePop();
		}
	}

	void Application::OnRender()
	{
		// Dockspace setup
		{
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;
			ImGuiViewport* viewport = ImGui::GetMainViewport();

			ImGui::SetNextWindowPos( viewport->Pos );
			ImGui::SetNextWindowSize( viewport->Size );
			ImGui::SetNextWindowViewport( viewport->ID );
			ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
			ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );
			ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );

			ImGui::Begin( "DockSpace", nullptr, window_flags );

			ImGui::PopStyleVar( 3 );

			ImGuiID dockspace_id = ImGui::GetID( "dockspace" );
			ImGui::DockSpace( dockspace_id, ImVec2( 0.0f, 0.0f ), ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode );

			ImGui::End();
		}

		if( imgui_menu_enabled )
		{
			ImGui::Begin( "Application" );

			ImGui::Text( "FPS: %s", fps_string.c_str() );
			ImGui::Text( "Pos: %.2f %.2f %.2f", camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z );

			ImGui::SliderFloat( "Bloom threshold", &bloom_threshold, 0.0f, 100.0f );

			AddNodeTree( scene );

			ImGui::End();
		}
	}

	void Application::OnEvent( const WindowResizeEvent& e )
	{
		camera.SetAspectRatio( (float)e.width / e.height );

		// Re-build everything from scratch
		BuildRenderGraph();
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
			Entity light = world.CreateEntity();
			light.Add<LightComponent>()
				.SetRange( 2.5f );
			light.Add<TransformComponent>()
				.SetPosition( camera.GetPosition() );
			scene.AddChild( light );

			auto transform = light.Get<TransformComponent>();
			IDynamicObject* phys = Physics::CreateDynamicObject( camera.GetPosition(), { 0.0f, 0.0f, 0.0f } );
			phys->AddSphereShape( 0.05f );

			lights.push_back( light );
			lights_phys.push_back( phys );
		}
		else if( e.key == 'V' )
		{
			Entity light = world.CreateEntity();
			light.Add<LightComponent>()
				.SetRange( 2.5f );
			light.Add<TransformComponent>()
				.SetPosition( camera.GetPosition() );
			scene.AddChild( light );
		}
		else if( e.key == 'F' )
		{
			auto& l = flashlight.Get<LightComponent>();
			l.SetEnabled( !l.IsEnabled() );
			snd->Play( "Assets/click.wav" );
		}
		else if( e.key == 'P' )
		{
			for( size_t i = 0; i < lights.size(); i++ )
			{
				lights_phys[i]->AddLinearImpulse( { 0.0f, 10.0f, 0.0f } );
			}
		}
		else if( e.key == 'I' )
		{
			imgui_menu_enabled = !imgui_menu_enabled;
		}
		else if( e.key == 'R' )
		{
			auto result = Physics::RayCast( camera.GetPosition() + camera.GetLookAtVector() * 0.5f, camera.GetLookAtVector(), 500.0f, HIT_DYNAMICS );
			if( result.hit )
			{
				BAT_LOG( "HIT!" );
				IDynamicObject* dynamic_object = reinterpret_cast<IDynamicObject*>(result.object);
				dynamic_object->AddLinearImpulse( (dynamic_object->GetPosition() - camera.GetPosition()).Normalize() * 10.0f );
			}
		}
	}

	void Application::BuildRenderGraph()
	{
		rendergraph.Reset();

		int post_process_count = 0;
		if( bloom_enabled ) post_process_count++;
		if( motion_blur_enabled ) post_process_count++;

		// initialize resources
		// render texture to draw scene to
		if( post_process_count )
		{
			rendergraph.AddRenderTargetResource( "target",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT ) ) );

			// render texture 1 for bloom
			rendergraph.AddRenderTargetResource( "rt1",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth() / 2, wnd.GetHeight() / 2, TEX_FORMAT_R32G32B32A32_FLOAT ) ) );
			// render texture 2 for bloom & motion blur
			rendergraph.AddRenderTargetResource( "rt2",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth() / 2, wnd.GetHeight() / 2, TEX_FORMAT_R32G32B32A32_FLOAT ) ) );
		}
		else
		{
			rendergraph.AddRenderTargetResource( "target", gpu->GetBackbuffer() );
		}

		auto depth = std::unique_ptr<IDepthStencil>( gpu->CreateDepthStencil( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R24G8_TYPELESS ) );
		gpu->GetContext()->SetDepthStencil( depth.get() );
		rendergraph.AddDepthStencilResource( "depth", std::move( depth ) );

		rendergraph.AddTextureResource( "skybox", std::unique_ptr<ITexture>( gpu->CreateTexture( "Assets\\skybox.dds" ) ) );

		// add passes
		rendergraph.AddPass( "crt", std::make_unique<ClearRenderTargetPass>() );
		rendergraph.BindToResource( "crt.buffer", "target" );
		rendergraph.BindToResource( "crt.depth", "depth" );

		rendergraph.AddPass( "opaque", std::make_unique<OpaquePass>() );
		rendergraph.BindToResource( "opaque.dst", "target" );

		rendergraph.AddPass( "transparent", std::make_unique<TransparentPass>() );
		rendergraph.BindToResource( "transparent.dst", "target" );

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
