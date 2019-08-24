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

#include <WinBase.h>
using namespace Bat;

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	try
	{
		BAT_INIT_SYSTEM( Logger );
		BAT_INIT_SYSTEM( Networking );
		BAT_INIT_SYSTEM( JobSystem );
		BAT_INIT_SYSTEM( Physics );
		BAT_INIT_SYSTEM( COMInitialize );
#ifdef _DEBUG
		BAT_INIT_PER_FRAME_SYSTEM( FileWatchdog );
#endif

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

			BAT_SERVICE_SYSTEMS( dt );

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