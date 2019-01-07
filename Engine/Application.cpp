#include "PCH.h"
#include "Application.h"

#include "Common.h"
#include "Graphics.h"
#include "Window.h"
#include "SceneLoader.h"
#include "FileWatchdog.h"

#include "WindowEvents.h"
#include "KeyboardEvents.h"

namespace Bat
{
	Application::Application( Graphics& gfx, Window& wnd )
		:
		gfx( gfx ),
		wnd( wnd ),
		scene( SceneLoader::LoadScene( "Assets/Sponza/Sponza.gltf" ) ),
		camera( wnd.input, 100.0f )
	{
		gfx.SetActiveScene( &scene );
		gfx.SetActiveCamera( &camera );

		wnd.OnEventDispatched<WindowResizeEvent>( [this]( const WindowResizeEvent& e )
		{
			this->camera.SetAspectRatio( (float)e.width / e.height );
		} );

		light = scene.GetRootNode().AddLight( {} );

		ui_overlay = gfx.UI().CreateOverlay( 300, 100, { 10, 30 } );
		ui_overlay->LoadHTMLFromFile( "Assets/UI/test.html" );
	}

	Application::~Application()
	{
		gfx.UI().DeleteOverlay( ui_overlay );
	}

	void Application::OnUpdate( float deltatime )
	{
		if( wnd.input.IsKeyDown( 'C' ) )
		{
			light->SetPosition( camera.GetPosition() );
		}

		camera.Update( deltatime );

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
