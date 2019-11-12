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
#include "EntityTrace.h"
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
		camera( wnd.input, 2.0f, 100.0f ),
		physics_system( world )
	{
		SceneLoader loader;

		camera.SetAspectRatio( (float)wnd.GetWidth() / wnd.GetHeight() );

		scene.Set( world.CreateEntity() ); // Root entity;
		animators.emplace_back();
		size_t scene_index = scene.AddChild( loader.Load( "Assets/Ignore/Sponza/sponza.gltf", &animators.back() ) );

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

			player = world.CreateEntity();
			player.Add<TransformComponent>()
				.SetPosition( camera.GetPosition() )
				.SetRotation( camera.GetRotation() );
			player.Add<PhysicsComponent>( PhysicsObjectType::DYNAMIC )
				.SetKinematic( true )
				.AddSphereShape( 0.05f );

			Entity scene_ent = scene.GetChild( scene_index ).Get();
			scene_ent.Add<TransformComponent>()
				.SetScale( 0.01f );
			//scene_ent.Add<PhysicsComponent>( PhysicsObjectType::STATIC )
			//	.AddMeshShape();
		}

		wnd.input.AddEventListener<KeyPressedEvent>( *this );

		g_Console.AddCommand( "load_scene", [&scene = scene, &cam = camera, &gfx = gfx, &animators = animators]( const CommandArgs_t& args )
		{
			std::string filepath = std::string( args[1] );
			if( !std::filesystem::exists( filepath ) )
			{
				BAT_WARN( "Invalid file path '%s'", filepath );
			}

			SceneLoader loader;
			animators.emplace_back();
			scene = loader.Load( filepath, &animators.back() );
			gfx.SetActiveScene( &scene );
		} );

		g_Console.AddCommand( "add_model", [&scene = scene, &cam = camera, &animators = animators]( const CommandArgs_t& args )
		{
			std::string filepath = std::string( args[1] );
			if( !std::filesystem::exists( filepath ) )
			{
				BAT_WARN( "Invalid file path '%s'", filepath );
			}

			SceneLoader loader;
			animators.emplace_back();
			SceneNode new_node = loader.Load( filepath, &animators.back() );

			auto pos = cam.GetPosition();
			Entity new_model = new_node.Get();
			new_model.Ensure<TransformComponent>()
				.SetPosition( pos );

			scene.AddChild( new_node );
		} );

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
		wnd.input.AddEventListener<MouseButtonPressedEvent>( *this );
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

		player.Get<TransformComponent>()
			.SetPosition( camera.GetPosition() )
			.SetRotation( camera.GetRotation() );

		physics_system.Update( world, deltatime );
		for( MeshAnimator& animator : animators )
		{
			if( animator.GetNumAnimations() > 0 )
			{
				timestamp += deltatime * anim_timescale;
				if( timestamp >= animator.GetCurrentAnimation().duration )
				{
					timestamp = 0;
				}

				animator.SetCurrentAnimationIndex( selected_anim );
				animator.SetTimestamp( timestamp );
			}
		}
		anim_system.Update( world );

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
			if( e.Has<PhysicsComponent>() )
			{
				ImGui::Text( "Physics" );
			}
			if( e.Has<AnimationComponent>() )
			{
				ImGui::Text( "Animation" );
			}
			if( e.Has<TransformComponent>() )
			{
				const auto& t = e.Get<TransformComponent>();
				Vec3 pos = t.GetPosition();
				Vec3 rot = t.GetRotation();
				float scale = t.GetScale();

				std::string pos_text = Format( "Local Pos: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z );
				std::string rot_text = Format( "Local Rot: (%.2f, %.2f, %.2f)", rot.x, rot.y, rot.z );
				std::string scale_text = Format( "Local Scale: %.2f", scale );
			
				if( ImGui::TreeNode( "Transform" ) )
				{
					ImGui::Text( pos_text.c_str() );
					ImGui::Text( rot_text.c_str() );
					ImGui::Text( scale_text.c_str() );

					ImGui::TreePop();
				}
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

			for( MeshAnimator& animator : animators )
			{
				if( animator.GetNumAnimations() > 0 )
				{
					std::vector<const char*> animation_names;
					for( size_t i = 0; i < animator.GetNumAnimations(); i++ )
					{
						animation_names.push_back( animator.GetAnimation( i ).name.c_str() );
					}
					ImGui::Combo( "Animation", &selected_anim, animation_names.data(), animator.GetNumAnimations() );
					ImGui::SliderFloat( "Animation timestamp", &timestamp, 0.0f, animator.GetAnimation( selected_anim ).duration );
					ImGui::SliderFloat( "Animation timescale", &anim_timescale, 1.0f, 100.0f );
				}
			}
			
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
			//light.Add<PhysicsComponent>( PhysicsObjectType::DYNAMIC )
			//	.AddSphereShape( 0.05f );
			scene.AddChild( light );
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
			for( Entity ent : world )
			{
				if( ent.Has<LightComponent>() && ent.Has<PhysicsComponent>() )
				{
					ent.Get<PhysicsComponent>().AddLinearImpulse( { 0.0f, 10.0f, 0.0f } );
				}
			}
		}
		else if( e.key == 'I' )
		{
			imgui_menu_enabled = !imgui_menu_enabled;
		}
		else if( e.key == 'R' )
		{
			auto result = EntityTrace::RayCast( camera.GetPosition() + camera.GetLookAtVector() * 0.5f, camera.GetLookAtVector(), 500.0f, HIT_DYNAMICS );
			if( result.hit )
			{
				Entity hit_ent = result.entity;
				BAT_LOG( "HIT! Entity: %i", hit_ent.GetId().GetIndex() );
				const auto& t = hit_ent.Get<TransformComponent>();
				auto& phys = hit_ent.Get<PhysicsComponent>();
				phys.AddLinearImpulse( (t.GetPosition() - camera.GetPosition()).Normalize() * 10.0f );
			}
		}
	}

	void Application::OnEvent( const MouseButtonPressedEvent& e )
	{
		if( e.button == Input::MouseButton::Left )
		{
			DirectX::XMMATRIX inv_proj = DirectX::XMMatrixInverse( nullptr, camera.GetProjectionMatrix() );
			DirectX::XMMATRIX inv_view = DirectX::XMMatrixInverse( nullptr, camera.GetViewMatrix() );

			float x = (2.0f * e.pos.x) / wnd.GetWidth() - 1.0f;
			float y = 1.0f - (2.0f * e.pos.y) / wnd.GetHeight();
			Vec4 clip = { x, y, 1.0f, -1.0f };
			Vec4 eye = DirectX::XMVector4Transform( clip, inv_proj );
			eye.z = 1.0f;
			eye.w = 0.0f;
			Vec4 world = DirectX::XMVector4Transform( eye, inv_view );
			Vec3 ray = Vec3( world.x, world.y, world.z ).Normalized();

			auto result = EntityTrace::RayCast( camera.GetPosition() + ray * 0.5f, ray, 500.0f, HIT_DYNAMICS );
			if( result.hit )
			{
				Entity hit_ent = result.entity;
				BAT_LOG( "HIT! Entity: %i", hit_ent.GetId().GetIndex() );
				const auto& t = hit_ent.Get<TransformComponent>();
				auto& phys = hit_ent.Get<PhysicsComponent>();
				phys.AddLinearImpulse( (t.GetPosition() - camera.GetPosition()).Normalize() * 10.0f );
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
