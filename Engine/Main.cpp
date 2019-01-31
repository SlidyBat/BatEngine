#include "PCH.h"

#include "COMInitialize.h"
#include "COMException.h"
#include "FrameTimer.h"
#include "Globals.h"
#include "JobSystem.h"
#include "ResourceManager.h"
#include "Window.h"
#include "Graphics.h"
#include "FileWatchdog.h"
#include "Application.h"
#include <spdlog/fmt/fmt.h>

#include "Core/Entity.h"

using namespace Bat;

struct PositionComponent : Component<PositionComponent>
{
	PositionComponent( float x, float y, float z )
		:
		x( x ),
		y( y ),
		z( z )
	{}

	float x, y, z;
};

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow )
{
	try
	{
		COMInitialize coinit;
		Logger::Init();
		BAT_LOG( "Initialized logger" );
		JobSystem::Initialize();
		BAT_TRACE("Initialized job system");
		FileWatchdog::Initialize();
		BAT_TRACE( "Initialized file watchdog" );

		Window wnd( { 50, 50 }, Graphics::InitialScreenWidth, Graphics::InitialScreenHeight, "Bat Engine", Graphics::FullScreen );
		BAT_TRACE( "Initialized window" );
		Graphics gfx( wnd );
		BAT_TRACE( "Initialized graphics" );

		BAT_LOG( "Cores available: {}", std::thread::hardware_concurrency() );

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

	FileWatchdog::Shutdown();
	ResourceManager::CleanUp();

	return 0;
}