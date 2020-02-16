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
#include "GraphicsConvert.h"
#include "Passes/ClearRenderTargetPass.h"
#include "Passes/SkyboxPass.h"
#include "Passes/BloomPass.h"
#include "Passes/ToneMappingPass.h"
#include "Passes/MotionBlurPass.h"
#include "Passes/OpaquePass.h"
#include "Passes/TransparentPass.h"
#include "Passes/DrawLightsPass.h"
#include "Passes/ShadowPass.h"
#include "Passes/MsaaResolvePass.h"
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
			.SetScale( 0.5f );

		// Fire
		if( false )
		{
			Entity emitter_test = world.CreateEntity();
			emitter_test.Add<TransformComponent>()
				.SetPosition( { 0.0f, 0.0f, 0.0f } );
			emitter_test.Add<HierarchyComponent>();
			auto& emitter = emitter_test.Add<ParticleEmitterComponent>( ResourceManager::GetTexture( "Assets/Ignore/particles/fire_02.png" ) );
			emitter.particles_per_sec = 50.0f;
			emitter.lifetime = 2.0f;
			emitter.start_scale = 0.32f;
			emitter.end_scale = 0.35f;
			emitter.gradient.AddStop( Colour( 237, 237, 0 ), 0.1f )
				.AddStop( Colour( 255, 0, 0 ), 0.5f );
			emitter.start_alpha = 0.08f;
			emitter.end_alpha = 0.0f;
			emitter.force_multiplier = 0.0f;
			emitter.motion_blur = 8.0f;
			emitter.normal = { 0.0f, 0.3f, 0.0f };
			emitter.rand_velocity_range = { 0.0f, 0.0f, 0.0f };

			scene.AddChild( emitter_test );
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
			.SetEnabled( false )
			.AddFlag( LightFlags::EMIT_SHADOWS );
		sun.Add<TransformComponent>()
			.SetRotation( 90.0f, 0.0f, 0.0f );
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
		flashlight.Get<TransformComponent>()
			.SetPosition( camera.GetPosition() )
			.SetRotation( camera.GetRotation() );
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

	static void AddNodeTree( SceneNode* parent_node, SceneNode& node )
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
				AddNodeTree( &node, node.GetChild( i ) );
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
			if( e.Has<ParticleEmitterComponent>() )
			{
				auto& emitter = e.Get<ParticleEmitterComponent>();

				if( ImGui::TreeNode( "Particle Emitter" ) )
				{
					ImGui::Text( "%i Particles", emitter.num_particles );
					if( ImGui::ImageButton( emitter.texture->Get()->GetImpl(), { 50, 50 } ) )
					{
						auto path = FileDialog::Open( "Assets" );
						if( path )
						{
							auto texture = ResourceManager::GetTexture( path->string() );
							if( texture ) emitter.texture = std::move( texture );
						}
					}
					emitter.gradient.DoImGuiButton();
					ImGui::DragFloat( "Particles/Sec", &emitter.particles_per_sec, 0.1f, 0.0f, 1000.0f );
					ImGui::DragFloat( "Lifetime", &emitter.lifetime, 0.01f, 0.0f, 10.0f );
					ImGui::DragFloat( "Start Alpha", &emitter.start_alpha, 0.01f, 0.0f, 1.0f );
					ImGui::DragFloat( "End Alpha", &emitter.end_alpha, 0.01f, 0.0f, 1.0f );
					ImGui::DragFloat( "Start Scale", &emitter.start_scale, 0.01f, 0.0f, 2.0f );
					ImGui::DragFloat( "End Scale", &emitter.end_scale, 0.01f, 0.0f, 2.0f );
					ImGui::DragFloat3( "Force", (float*)&emitter.force, 0.01f, -10.0f, 10.0f );
					ImGui::DragFloat( "Force Multiplier", &emitter.force_multiplier, 0.1f, 0.0f, 10.0f );
					ImGui::DragFloat3( "Normal", (float*)&emitter.normal, 0.01f, -5.0f, 5.0f );
					ImGui::DragFloat( "Rot. Velocity Range", &emitter.rand_rot_velocity_range, 0.01f, 0.0f, 100.0f );
					ImGui::DragFloat3( "Velocity Range", (float*)&emitter.rand_velocity_range, 0.01f, 0.0f, 100.0f );
					ImGui::DragFloat( "Motion Blur", &emitter.motion_blur, 1.0f, 0.0f, 100.0f );
					ImGui::TreePop();
				}
			}

			if( parent_node && ImGui::Button( "Delete" ) )
			{
				parent_node->RemoveChild( node.Get() );
				world.DestroyEntity( e );
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
		Vec3 rot = camera.GetRotation();
		auto rotstr = Format( "Rot: %.2f %.2f %.2f", rot.x, rot.y, rot.z );
		DebugDraw::Text( rotstr, { 10, 20 } );

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

					changed |= ImGui::Checkbox( "Skybox", &skybox_enabled );
					if( skybox_enabled && rendergraph.GetPassByName( "skybox" ) )
					{
						auto skybox = static_cast<SkyboxPass*>( rendergraph.GetPassByName( "skybox" ) );
						bool dynamic_sky = skybox->IsDynamicSkyEnabled();

						if( ImGui::Checkbox( "Dynamic sky", &dynamic_sky ) )
						{
							skybox->SetDynamicSkyEnabled( dynamic_sky );
						}

						if( !dynamic_sky )
						{
							if( ImGui::Button( "Load texture" ) )
							{
								auto path = FileDialog::Open( "Assets" );
								if( path )
								{
									skybox_tex = path->string();
									changed = true;
								}
							}
						}
					}

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

					changed |= ImGui::Checkbox( "MSAA", &msaa_enabled );
					if( msaa_enabled )
					{
						static int current_msaa_sample = 2;
						static const char* samples[] = { "1", "2", "4", "8" };
						if( ImGui::Combo( "MSAA Samples", &current_msaa_sample, samples, ARRAYSIZE( samples ) ) )
						{
							changed = true;
							msaa_samples = std::stoi( samples[current_msaa_sample] );
						}
					}

					if( changed )
					{
						BuildRenderGraph();
					}
				}

				if( ImGui::CollapsingHeader( "Scene Hierarchy" ) )
				{
					AddNodeTree( nullptr, scene );
				
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
		else if( e.key == 'E' )
		{
			Entity spotlight = world.CreateEntity();
			spotlight.Add<LightComponent>()
				.SetType( LightType::SPOT )
				.SetSpotlightAngle( Math::DegToRad( 45.0f ) )
				.AddFlag( LightFlags::EMIT_SHADOWS );
			spotlight.Add<TransformComponent>()
				.SetPosition( camera.GetPosition() )
				.SetRotation( camera.GetRotation() );
			scene.AddChild( spotlight );
		}
		else if( e.key == 'Q' )
		{
			auto& t = sun.Get<TransformComponent>();
			t.SetRotation( camera.GetRotation() );
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
		IGPUContext* pContext = gpu->GetContext();

		ScratchRenderTarget::Clear();
		ShaderManager::BindShaderGlobals( gfx.GetActiveCamera(), { (float)wnd.GetWidth(), (float)wnd.GetHeight() }, pContext );

		rendergraph.Reset();

		int post_process_count = 0;
		if( bloom_enabled ) post_process_count++;
		if( motion_blur_enabled ) post_process_count++;
		if( tonemapping_enabled ) post_process_count++;

		size_t ms_samples = msaa_enabled ? (size_t)msaa_samples : 1;
		MsaaQuality ms_quality = msaa_enabled ? MsaaQuality::STANDARD_PATTERN : MsaaQuality::NONE;

		// initialize resources
		// render texture to draw scene to
		if( post_process_count )
		{
			if( msaa_enabled )
			{
				rendergraph.AddRenderTargetResource( "target",
					std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT, ms_quality, ms_samples ) ) );
				rendergraph.AddRenderTargetResource( "target2",
					std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT ) ) );
				rendergraph.AddRenderTargetResource( "resolve_target",
					std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT ) ) );
			}
			else
			{
				rendergraph.AddRenderTargetResource( "target",
					std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT, ms_quality, ms_samples ) ) );
				rendergraph.AddRenderTargetResource( "target2",
					std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT, ms_quality, ms_samples ) ) );
			}
		}
		else
		{
			if( msaa_enabled )
			{
				rendergraph.AddRenderTargetResource( "target",
					std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT, ms_quality, ms_samples, TexFlags::NO_SHADER_BIND ) ) );
				rendergraph.AddRenderTargetResource( "resolve_target", gpu->GetBackbuffer() );
			}
			else
			{
				rendergraph.AddRenderTargetResource( "target", gpu->GetBackbuffer() );
			}
		}

		auto depth = std::unique_ptr<IDepthStencil>( gpu->CreateDepthStencil( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R24G8_TYPELESS, 1, ms_quality, ms_samples, TexFlags::NO_SHADER_BIND ) );
		rendergraph.AddDepthStencilResource( "depth", std::move( depth ) );

		auto hdrmap = std::unique_ptr<ITexture>( gpu->CreateTexture( skybox_tex, TexFlags::NO_GEN_MIPS ) );
		auto cubemap = std::unique_ptr<IRenderTarget>( GraphicsConvert::EquirectangularToCubemap( pContext, hdrmap.get(), 512, 512 ) );
		rendergraph.AddTextureResource( "skybox", std::unique_ptr<ITexture>( cubemap->AsTexture() ) );

		// add passes
		rendergraph.AddPass( "crt", std::make_unique<ClearRenderTargetPass>() );
		rendergraph.BindToResource( "crt.buffer", "target" );
		rendergraph.BindToResource( "crt.depth", "depth" );

		rendergraph.AddPass( "shadows", std::make_unique<ShadowPass>() );

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

		if( skybox_enabled )
		{
			rendergraph.BindToResource( "draw_lights.dst", "target" );

			rendergraph.AddPass( "skybox", std::make_unique<SkyboxPass>() );
			rendergraph.BindToResource( "skybox.skyboxtex", "skybox" );
		}
		if( !post_process_count )
		{
			if( msaa_enabled )
			{
				rendergraph.AddPass( "resolve", std::make_unique<MsaaResolvePass>() );
				rendergraph.BindToResource( "resolve.src", "target" );
				rendergraph.BindToResource( "resolve.dst", "resolve_target" );
			}
			else if( skybox_enabled )
			{
				rendergraph.MarkOutput( "skybox.dst" );
			}
			else
			{
				rendergraph.MarkOutput( "draw_lights.dst" );
			}
		}
		else
		{
			if( skybox_enabled )
			{
				rendergraph.BindToResource( "skybox.dst", "target" );
			}
			else
			{
				rendergraph.BindToResource( "draw_lights.dst", "target" );
			}
			std::string input_rt = "target";

			if( msaa_enabled )
			{
				rendergraph.AddPass( "resolve", std::make_unique<MsaaResolvePass>() );
				rendergraph.BindToResource( "resolve.src", input_rt );
				rendergraph.BindToResource( "resolve.dst", "resolve_target" );
				input_rt = "resolve_target";
			}

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
