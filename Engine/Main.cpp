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
#include "Application.h"

using namespace Bat;

static void InitializeSubsystems()
{
	Logger::Initialize();
	BAT_TRACE( "Initialized logger" );
	Networking::Initialize();
	BAT_TRACE( "Initialized networking" );
	JobSystem::Initialize();
	BAT_TRACE("Initialized job system");
	FileWatchdog::Initialize();
	BAT_TRACE( "Initialized file watchdog" );
}

static void DestroySubsystems()
{
	FileWatchdog::Shutdown();
	BAT_TRACE( "Shut down file watchdog" );
	JobSystem::Shutdown();
	BAT_TRACE("Shut down job system");
	Networking::Shutdown();
	BAT_TRACE( "Shut down networking" );
	Logger::Shutdown();
	BAT_TRACE( "Shut down logger" );
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow )
{
	try
	{
		COMInitialize coinit;
		InitializeSubsystems();

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
			fmt::format( "{}\nFile: {}\nSource: {}\nDescription: {}\n",
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
	DestroySubsystems();

	return 0;
}