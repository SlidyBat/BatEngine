#include "Demo.h"

#include <Core/Entry.h>
#include <filesystem>

#include "MoveableCharacter.h"
#include "AiCharacter.h"
#include "Sound.h"
#include "TypeToString.h"

using namespace Bat;

Bat::IApplication* Bat::CreateApplication( int argc, char* argv[], Renderer& gfx, Window& wnd )
{
	return new Demo( gfx, wnd );
}

static MoveableCharacter player;
static AiCharacter ai;

BAT_REFLECT_EXTERNAL_BEGIN( Vec3 );
	BAT_REFLECT_MEMBER( x );
	BAT_REFLECT_MEMBER( y );
	BAT_REFLECT_MEMBER( z );
BAT_REFLECT_END();

Demo::Demo( Renderer& gfx, Window& wnd )
	:
	gfx( gfx ),
	wnd( wnd ),
	camera( wnd.input, 2.0f, 1.0f ),
	physics_system( world ),
	controller_system( world )
{
	InitializeSound();

	wnd.SetIcon( "Assets/slidy.ico" );

	SceneLoader loader;

	camera.SetAspectRatio( (float)wnd.GetWidth() / wnd.GetHeight() );

	scene.Set( world.CreateEntity() ); // Root entity
	scale_node = scene.AddChild( world.CreateEntity() );
	scale_node->Get().Get<TransformComponent>()
		.SetScale( 0.5f );
	scale_node->AddChild( loader.Load( "Assets/Ignore/Sponza/sponza.gltf" ) );
	Entity floor = scene.AddChild( world.CreateEntity() )->Get();
	floor.Get<TransformComponent>()
		.SetPosition( { 0.0f, -2.0f, 0.0f } )
		.SetRotation( { 0.0f, 0.0f, 90.0f } );
	floor.Add<PhysicsComponent>( PhysicsObjectType::STATIC )
		.AddPlaneShape();

	BAT_LOG( TypeToString::DumpComponents( floor ) );

	navmesh_system.Bake();

	player.Initialize( scene, { 0.0f, 1.0f, 0.0f } );
	ai.Initialize( scene, { 1.0f, 0.5f, 2.0f }, navmesh_system, player.GetEntity() );

	// Fire
	if( false )
	{
		Entity emitter_test = world.CreateEntity();
		scene.AddChild( emitter_test );
		emitter_test.Get<TransformComponent>()
			.SetPosition( { 0.0f, 0.0f, 0.0f } );
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
	}

	flashlight = world.CreateEntity();
	scene.AddChild( flashlight );
	flashlight.Add<LightComponent>()
		.SetType( LightType::SPOT )
		.SetSpotlightAngle( 0.5f );

	sun = world.CreateEntity();
	scene.AddChild( sun );
	sun.Get<TransformComponent>()
		.SetRotation( 90.0f, 0.0f, 0.0f );
	sun.Add<LightComponent>()
		.SetType( LightType::DIRECTIONAL )
		.SetEnabled( false )
		.AddFlag( LightFlags::EMIT_SHADOWS );

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

		scale_node->GetFirstChild()->Get().Add<PhysicsComponent>( PhysicsObjectType::STATIC )
			.AddMeshShape();
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

	g_Console.AddCommand( "sun_toggle", [&sun = sun]( const CommandArgs_t& args )
	{
		LightComponent& l = sun.Get<LightComponent>();
		l.SetEnabled( !l.IsEnabled() );
	} );

	//EventDispatcher::OnGlobalEventDispatched<PhysicsTriggerStartTouchEvent>( []( const PhysicsTriggerStartTouchEvent& e )
	//{
	//	BAT_LOG( "Entered trigger" );
	//} );
	//EventDispatcher::OnGlobalEventDispatched<PhysicsTriggerEndTouchEvent>( []( const PhysicsTriggerEndTouchEvent& e )
	//{
	//	BAT_LOG( "Exited trigger" );
	//} );
	//EventDispatcher::OnGlobalEventDispatched<PhysicsObjectStartTouchEvent>( []( const PhysicsObjectStartTouchEvent& e )
	//{
	//	BAT_LOG( "Touch!" );
	//} );

	wnd.AddEventListener<WindowResizeEvent>( *this );
	wnd.input.AddEventListener<MouseButtonPressedEvent>( *this );
}

Demo::~Demo()
{
	delete snd;
}

void Demo::OnUpdate( float deltatime )
{
	elapsed_time += deltatime;
	fps_counter += 1;
	if( elapsed_time > 1.0f )
	{
		fps_string = std::to_string( fps_counter );
		fps_counter = 0;
		elapsed_time -= 1.0f;
	}

	player.Update( wnd.input, deltatime );
	camera.SetPosition( player.GetPosition() );
	camera.SetRotation( player.GetRotation() );

	flashlight.Get<TransformComponent>()
		.SetPosition( camera.GetPosition() )
		.SetRotation( camera.GetRotation() );
	snd->SetListenerPosition( camera.GetPosition(), camera.GetLookAtVector() );

	physics_system.Update( world, deltatime );
	anim_system.Update( world, deltatime );
	particle_system.Update( world, deltatime );
	behaviour_system.Update( world );
	controller_system.Update( world, deltatime );

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
		for( SceneNode& child : node )
		{
			AddNodeTree( &node, child );
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
			Vec3 pos = t.GetLocalPosition();
			Vec3 rot = t.GetLocalRotation();
			float scale = t.GetLocalScale();

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

void Demo::OnRender()
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
				renderbuilder.DrawSettings( gfx, wnd, &rendergraph );
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

	Vec3 ai_pos = ai.GetPosition();
	DebugDraw::Box( ai_pos - Vec3{ 1.0f, 1.0f, 1.0f } *0.1f, ai_pos + Vec3{ 1.0f, 1.0f, 1.0f } *0.1f, Colours::Red );

	if( draw_navmesh )
	{
		navmesh_system.Draw( 0 );
	}
}

void Demo::OnEvent( const WindowResizeEvent& e )
{
	camera.SetAspectRatio( (float)e.width / e.height );

	// Re-build everything from scratch
	BuildRenderGraph();
}

void Demo::OnEvent( const KeyPressedEvent& e )
{
	if( e.key == VK_OEM_3 )
	{
		BAT_LOG( "Toggling console" );
		g_Console.SetVisible( !g_Console.IsVisible() );
	}
	else if( e.key == 'B' )
	{
		// toggle bloom
		renderbuilder.bloom_enabled = !renderbuilder.bloom_enabled;
		// re-build render graph
		BuildRenderGraph();
	}
	else if( e.key == 'M' )
	{
		// toggle motion blur
		renderbuilder.motion_blur_enabled = !renderbuilder.motion_blur_enabled;
		// re-build render graoh
		BuildRenderGraph();
	}
	else if( e.key == 'C' )
	{
		Entity light = world.CreateEntity();
		scene.AddChild( light );
		light.Get<TransformComponent>()
			.SetPosition( camera.GetPosition() );
		light.Add<LightComponent>()
			.SetRange( 2.5f );
		//light.Add<PhysicsComponent>( PhysicsObjectType::DYNAMIC )
		//	.AddSphereShape( 0.05f );
	}
	else if( e.key == 'V' )
	{
		Entity light = world.CreateEntity();
		scene.AddChild( light );
		light.Get<TransformComponent>()
			.SetPosition( camera.GetPosition() );
		light.Add<LightComponent>()
			.SetRange( 2.5f );
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
		draw_navmesh = !draw_navmesh;
	}
	else if( e.key == 'X' )
	{
		physics_simulate = !physics_simulate;
	}
	else if( e.key == 'E' )
	{
		Entity spotlight = world.CreateEntity();
		spotlight.Get<TransformComponent>()
			.SetPosition( camera.GetPosition() )
			.SetRotation( camera.GetRotation() );
		scene.AddChild( spotlight );
		spotlight.Add<LightComponent>()
			.SetType( LightType::SPOT )
			.SetSpotlightAngle( Math::DegToRad( 45.0f ) )
			.AddFlag( LightFlags::EMIT_SHADOWS );
	}
	else if( e.key == 'Q' )
	{
		auto& t = sun.Get<TransformComponent>();
		t.SetRotation( camera.GetRotation() );
	}
}

void Demo::OnEvent( const MouseButtonPressedEvent& e )
{
	if( e.button == Input::MouseButton::Left )
	{
	}
}

void Demo::LoadModel( const std::string& filename )
{
	std::string filepath = std::string( filename );
	if( !std::filesystem::exists( filepath ) )
	{
		BAT_WARN( "Invalid file path '%s'", filepath );
	}

	SceneLoader loader;
	SceneNode new_node = loader.Load( filepath );

	const Vec3& pos = camera.GetPosition();
	SceneNode* pos_node = scale_node->AddChild( world.CreateEntity() );
	pos_node->AddChild( std::move( new_node ) );
	pos_node->Get().Get<TransformComponent>()
		.SetPosition( pos );
}

void Demo::BuildRenderGraph()
{
	renderbuilder.Make( gfx, wnd, &rendergraph );
}
