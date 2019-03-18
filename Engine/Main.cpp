#include "PCH.h"

#include "COMInitialize.h"
#include "COMException.h"
#include "FrameTimer.h"
#include "Globals.h"
#include "Networking.h"
#include "JobSystem.h"
#include "ResourceManager.h"
#include "Window.h"
#include "Graphics.h"
#include "FileWatchdog.h"
#include "Physics.h"
#include "Application.h"
#include "EngineSystems.h"

using namespace Bat;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow )
{
	try
	{
		BAT_INIT_SYSTEM( Logger );
		BAT_INIT_SYSTEM( Networking );
		BAT_INIT_SYSTEM( JobSystem );
		BAT_INIT_SYSTEM( FileWatchdog );
		BAT_INIT_SYSTEM( Physics );
		BAT_INIT_SYSTEM( COMInitialize );

		Window wnd( { 50, 50 }, Graphics::InitialScreenWidth, Graphics::InitialScreenHeight, "Bat Engine", Graphics::FullScreen );
		BAT_TRACE( "Initialized window" );
		Graphics gfx( wnd );
		BAT_TRACE( "Initialized graphics" );

		FrameTimer ft;

		Application app( gfx, wnd );
		while( wnd.ProcessMessage() )
		{
			float dt = ft.Mark();
			g_pGlobals->deltatime = dt;
			g_pGlobals->elapsed_time += dt;

			app.OnUpdate( dt );

			gfx.BeginFrame();

			app.OnRender();
			g_Console.Draw("Bat Engine Console");

			gfx.EndFrame();
		}
	}
	catch( const std::exception& e )
	{
		MessageBox( NULL, e.what(), "Error", MB_ICONERROR | MB_OK );
	}
	catch( const _com_error& e )
	{
		MessageBox( NULL,
			Bat::Format( "%s\nFile: %s\nSource: %s\nDescription: %s\n",
				e.ErrorMessage(),
				(const char*)e.HelpFile(),
				(const char*)e.Source(),
				(const char*)e.Description()
			).c_str(),
			"COM Error",
			MB_ICONERROR
		);
	}

	ResourceManager::CleanUp();

	return 0;
}