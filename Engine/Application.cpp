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
#include "Globals.h"

namespace Bat
{
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
		gfx.SetSkybox( ResourceManager::GetTexture( "Assets\\skybox.dds" ) );

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
