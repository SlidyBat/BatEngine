#include "PCH.h"

#include "COMInitialize.h"
#include "COMException.h"
#include "FrameTimer.h"
#include "Globals.h"
#include "JobSystem.h"
#include "ResourceManager.h"
#include "Window.h"
#include "Graphics.h"
#include "Application.h"

using namespace Bat;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow )
{
	try
	{
		COMInitialize coinit;
		Logger::Init();
		BAT_LOG( "Initialized logger" );
		JobSystem::Initialize();
		BAT_TRACE("Initialized job system");
		
		Window wnd( { 50, 50 }, Graphics::InitialScreenWidth, Graphics::InitialScreenHeight, "Bat Engine", Graphics::FullScreen );
		BAT_TRACE( "Initialized window" );
		Graphics gfx( wnd );
		BAT_TRACE( "Initialized graphics" );

		BAT_LOG( "Cores available: {}", std::thread::hardware_concurrency() );

		FrameTimer ft;

		Application app( wnd );
		while( wnd.ProcessMessage() )
		{
			float dt = ft.Mark();
			g_pGlobals->deltatime = dt;
			g_pGlobals->elapsed_time += dt;

			app.OnUpdate( dt );

			gfx.BeginFrame();
			
			app.OnRender();


			gfx.EndFrame();
		}
	}
	catch( const std::exception& e )
	{
		MessageBox( NULL, e.what(), "Error", MB_ICONWARNING | MB_OK );
	}

	ResourceManager::CleanUp();

	return 0;
}