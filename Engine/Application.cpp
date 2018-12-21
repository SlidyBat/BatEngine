#include "PCH.h"
#include "Application.h"

#include "KeyboardEvents.h"
#include "IGraphics.h"
#include "Window.h"
#include "SceneLoader.h"

namespace Bat
{
	Application::Application( Window& wnd )
		:
		wnd( wnd ),
		scene( SceneLoader::LoadScene( "Assets/Car/scene.gltf" ) ),
		camera( 200.0f )
	{
		g_pGfx->SetScene( &scene );
		g_pGfx->SetCamera( &camera );

		light = scene.GetRootNode().AddLight( {} );
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
		g_pGfx->DrawText( Bat::StringToWide( fps_string ).c_str(), DirectX::XMFLOAT2{ 15.0f, 15.0f } );
	}
}
