#include "PCH.h"
#include "Application.h"

#include "Common.h"
#include "Graphics.h"
#include "Window.h"
#include "SceneLoader.h"
#include "FileWatchdog.h"

namespace Bat
{
	Application::Application( Graphics& gfx, Window& wnd )
		:
		gfx( gfx ),
		wnd( wnd ),
		scene( SceneLoader::LoadScene( "Assets/Car/scene.gltf" ) ),
		camera( 200.0f )
	{
		gfx.SetActiveScene( &scene );
		gfx.SetActiveCamera( &camera );

		light = scene.GetRootNode().AddLight( {} );

FileWatchdog::AddFileChangeListener( "test.txt", BIND_MEM_FN( Application::OnFileChanged ) );
FileWatchdog::AddFileChangeListener( "test2.txt", BIND_MEM_FN( Application::OnFileChanged ) );
	}

	void Application::OnUpdate( float deltatime )
	{
		if( wnd.input.IsKeyPressed( 'C' ) )
		{
			light->SetPosition( camera.GetPosition() );
		}

		camera.Update( wnd.input, deltatime );

		elapsed_time += deltatime;
		fps_counter += 1;
		if( elapsed_time > 1.0f )
		{
			fps_string = "FPS: " + std::to_string( fps_counter );
			fps_counter = 0;
			elapsed_time -= 1.0f;
		}
	}

	void Application::OnRender()
	{
		gfx.DrawText( Bat::StringToWide( fps_string ).c_str(), DirectX::XMFLOAT2{ 15.0f, 15.0f } );
	}

	void Application::OnFileChanged( const std::string& filename )
	{
		BAT_LOG( "'{}' changed", filename );
	}
}
