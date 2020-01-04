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
#include "EntityTrace.h"
#include "Passes/ClearRenderTargetPass.h"
#include "Passes/SkyboxPass.h"
#include "Passes/BloomPass.h"
#include "Passes/ToneMappingPass.h"
#include "Passes/MotionBlurPass.h"
#include "Passes/OpaquePass.h"
#include "Passes/TransparentPass.h"
#include "Passes/DrawLightsPass.h"
#include "DebugDraw.h"
#include "ScratchRenderTarget.h"
#include "FileDialog.h"

namespace Bat
{
	Application::Application( Graphics& gfx, Window& wnd )
		:
		gfx( gfx ),
		wnd( wnd ),
		camera( wnd.input, 2.0f, 1.0f ),
		physics_system( world )
	{
		world.EnsureEntityCapacity( 1000005 );
		SceneLoader loader;

		camera.SetAspectRatio( (float)wnd.GetWidth() / wnd.GetHeight() );

		scene.Set( world.CreateEntity() ); // Root entity
		scale_index = scene.AddChild( world.CreateEntity() );
		SceneNode& scale_node = scene.GetChild( scale_index );
		scale_node.AddChild( loader.Load( "Assets/Ignore/Sponza/sponza.gltf" ) );
		Entity floor = world.CreateEntity();
		floor.Add<TransformComponent>()
			.SetPosition( { 0.0f, -2.0f, 0.0f } )
			.SetRotation( { 0.0f, 90.0f, 0.0f } );
		floor.Add<PhysicsComponent>( PhysicsObjectType::STATIC )
			.AddPlaneShape();

		scale_node.Get().Add<TransformComponent>()
			.SetScale( 0.01f );

		// Fire
		{
			Entity emitter_test = world.CreateEntity();
			emitter_test.Add<TransformComponent>()
				.SetPosition( { 0.0f, 0.0f, 0.0f } );
			emitter_test.Add<HierarchyComponent>();
			auto& emitter = emitter_test.Add<ParticleEmitterComponent>( ResourceManager::GetTexture( "Assets/Ignore/particles/fire_02.png" ) );
			emitter.particles_per_sec = 100.0f;
			emitter.lifetime = 5.0f;
			emitter.start_scale = 0.2f;
			emitter.end_scale = 0.15f;
			emitter.gradient.AddStop( Colour( 237, 237, 0 ), 0.1f )
				.AddStop( Colour( 255, 0, 0 ), 0.5f );
			emitter.start_alpha = 1.0f;
			emitter.end_alpha = 0.0f;
			emitter.gravity_multiplier = 0.0f;
		}
		// Smoke
		{
			Entity emitter_test = world.CreateEntity();
			emitter_test.Add<TransformComponent>()
				.SetPosition( { 0.0f, 0.0f, 0.0f } );
			emitter_test.Add<HierarchyComponent>();
			auto& emitter = emitter_test.Add<ParticleEmitterComponent>( ResourceManager::GetTexture( "Assets/Ignore/particles/smoke_01.png" ) );
			emitter.particles_per_sec = 50.0f;
			emitter.lifetime = 10.0f;
			emitter.start_scale = 0.2f;
			emitter.end_scale = 0.1f;
			emitter.gradient.AddStop( Colour( 128, 128, 128 ), 0.0f );
			emitter.start_alpha = 0.05f;
			emitter.end_alpha = 0.0f;
			emitter.gravity_multiplier = 0.0f;
		}

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

			//scene_ent.Add<PhysicsComponent>( PhysicsObjectType::STATIC )
			//	.AddMeshShape();
		}

		wnd.input.AddEventListener<KeyPressedEvent>( *this );

		g_Console.AddCommand( "load_scene", [&scene = scene, &cam = camera, &gfx = gfx]( const CommandArgs_t& args )
		{
			std::string filepath = std::string( args[1] );
			if( !std::filesystem::exists( filepath ) )
			{
				BAT_WARN( "Invalid file path '%s'", filepath );
			}

			SceneLoader loader;
			scene = loader.Load( filepath );
			gfx.SetActiveScene( &scene );
		} );

		g_Console.AddCommand( "add_model", [this]( const CommandArgs_t& args )
		{
			LoadModel( std::string( args[1] ) );
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
		anim_system.Update( world, deltatime );
		hier_system.Update( scene );
		particle_system.Update( world, deltatime );

		if( physics_simulate )
		{
			Physics::Simulate( deltatime );
		}
	}

	static void AddNodeTree( const SceneNode& node )
	{
		Entity e = node.Get();
		ImGui::PushID( std::to_string( e.GetId().Raw() ).c_str() );

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
				auto& model = e.Get<ModelComponent>();
				model.DoImGuiMenu();
			}
			if( e.Has<LightComponent>() )
			{
				auto& light = e.Get<LightComponent>();
				light.DoImGuiMenu();
			}
			if( e.Has<PhysicsComponent>() )
			{
				ImGui::Text( "Physics" );
			}
			if( e.Has<AnimationComponent>() )
			{
				if( ImGui::TreeNode( "Animation" ) )
				{
					auto& anim = e.Get<AnimationComponent>();

					for( size_t i = 0; i < anim.states.size(); )
					{
						AnimationState& state = anim.states[i];
						state.DoImGuiMenu();
						ImGui::PushID( state.GetClip() );
						if( ImGui::Button( "Delete" ) )
						{
							anim.states.erase( anim.states.begin() + i );
						}
						else
						{
							i++;
						}
						ImGui::PopID();
					}

					std::vector<AnimationClip*> clips;
					std::vector<const char*> clip_names;
					for( AnimationClip& clip : anim.clips )
					{
						bool exists = false;

						for( AnimationState& state : anim.states )
						{
							if( state.GetClip() == &clip )
							{
								exists = true;
								break;
							}
						}

						if( exists )
						{
							continue;
						}

						clips.push_back( &clip );
						clip_names.push_back( clip.name.c_str() );
					}

					static int selected_clip = 0;
					ImGui::Combo( "Clip", &selected_clip, clip_names.data(), (int)clip_names.size() );
					if( ImGui::Button( "Add clip" ) )
					{
						AnimationState new_state( clips[selected_clip] );
						anim.states.push_back( new_state );
						selected_clip = 0;
					}

					ImGui::TreePop();
				}
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

		ImGui::PopID();
	}

	void Application::OnRender()
	{
		Vec3 pos = camera.GetPosition();
		auto posstr = Format( "Pos: %.2f %.2f %.2f", pos.x, pos.y, pos.z );
		DebugDraw::Text( posstr, { 10, 10 } );

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
			if( ImGui::Begin( "Application" ) )
			{
				ImGui::Text( "FPS: %s", fps_string.c_str() );
				ImGui::Text( "Pos: %.2f %.2f %.2f", camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z );

				if( ImGui::CollapsingHeader( "Render Passes" ) )
				{
					bool changed = false;

					changed |= ImGui::Checkbox( "Opaque pass", &opaque_pass );
					changed |= ImGui::Checkbox( "Transparent pass", &transparent_pass );

					changed |= ImGui::Checkbox( "Bloom", &bloom_enabled );
					if( bloom_enabled )
					{
						if( ImGui::SliderFloat( "Bloom threshold", &bloom_threshold, 0.0f, 100.0f ) )
						{
							auto bloom = static_cast<BloomPass*>(rendergraph.GetPassByName( "bloom" ));
							bloom->SetThreshold( bloom_threshold );
						}
					}

					changed |= ImGui::Checkbox( "Motion blur", &motion_blur_enabled );
					changed |= ImGui::Checkbox( "Tonemapping", &tonemapping_enabled );
					if( tonemapping_enabled )
					{
						if( ImGui::SliderFloat( "Exposure", &exposure, 0.0f, 32.0f ) )
						{
							auto tm = static_cast<ToneMappingPass*>(rendergraph.GetPassByName( "tonemapping" ));
							tm->SetExposure( exposure );
						}
					}

					if( changed )
					{
						BuildRenderGraph();
					}
				}

				if( ImGui::CollapsingHeader( "Scene Hierarchy" ) )
				{
					AddNodeTree( scene );
				
					if( ImGui::Button( "Load model" ) )
					{
						auto path = FileDialog::Open( "Assets" );
						if( path )
						{
							LoadModel( path->string() );
						}
					}
				}

				ImGui::End();
			}
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
		else if( e.key == 'X' )
		{
			physics_simulate = !physics_simulate;
		}
	}

	void Application::OnEvent( const MouseButtonPressedEvent& e )
	{
		if( e.button == Input::MouseButton::Left )
		{
		}
	}

	void Application::LoadModel( const std::string& filename )
	{
		std::string filepath = std::string( filename );
		if( !std::filesystem::exists( filepath ) )
		{
			BAT_WARN( "Invalid file path '%s'", filepath );
		}

		SceneLoader loader;
		SceneNode new_node = loader.Load( filepath );

		const Vec3& pos = camera.GetPosition();
		SceneNode pos_node( world.CreateEntity() );
		pos_node.AddChild( new_node );
		pos_node.Get().Add<TransformComponent>()
			.SetPosition( pos );

		scene.GetChild( scale_index ).AddChild( pos_node );
	}

	void Application::BuildRenderGraph()
	{
		ScratchRenderTarget::Clear();

		rendergraph.Reset();

		int post_process_count = 0;
		if( bloom_enabled ) post_process_count++;
		if( motion_blur_enabled ) post_process_count++;
		if( tonemapping_enabled ) post_process_count++;

		// initialize resources
		// render texture to draw scene to
		if( post_process_count )
		{
			rendergraph.AddRenderTargetResource( "target",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT ) ) );
			rendergraph.AddRenderTargetResource( "target2",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT ) ) );
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

		if( opaque_pass )
		{
			rendergraph.AddPass( "opaque", std::make_unique<OpaquePass>() );
			rendergraph.BindToResource( "opaque.dst", "target" );
		}

		if( transparent_pass )
		{
			rendergraph.AddPass( "transparent", std::make_unique<TransparentPass>() );
			rendergraph.BindToResource( "transparent.dst", "target" );
		}

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
			std::string input_rt = "target";

			if( bloom_enabled )
			{
				post_process_count--;

				auto bloom = std::make_unique<BloomPass>();
				bloom->SetThreshold( bloom_threshold );

				rendergraph.AddPass( "bloom", std::move( bloom ) );
				rendergraph.BindToResource( "bloom.src", input_rt );
				if( !post_process_count )
				{
					rendergraph.MarkOutput( "bloom.dst" );
				}
				else
				{
					rendergraph.BindToResource( "bloom.dst", "target2" );
				}

				input_rt = "target2";
			}

			if( motion_blur_enabled )
			{
				post_process_count--;

				rendergraph.AddPass( "motionblur", std::make_unique<MotionBlurPass>() );
				rendergraph.BindToResource( "motionblur.src", input_rt );
				rendergraph.BindToResource( "motionblur.depth", "depth" );
				if( !post_process_count )
				{
					rendergraph.MarkOutput( "motionblur.dst" );
				}
				else
				{
					rendergraph.BindToResource( "motionblur.dst", "target" );
				}

				input_rt = "target";
			}

			if( tonemapping_enabled )
			{
				post_process_count--;

				auto tm = std::make_unique<ToneMappingPass>();
				tm->SetExposure( exposure );

				rendergraph.AddPass( "tonemapping", std::move( tm ) );
				rendergraph.BindToResource( "tonemapping.src", input_rt );

				rendergraph.MarkOutput( "tonemapping.dst" );
			}
		}
	}
}
