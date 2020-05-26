#include "Platform/BatWinAPI.h"
#include "Console.h"
#include "Platform/COMInitialize.h"
#include "Util/COMException.h"
#include "Util/FrameTimer.h"
#include "Globals.h"
#include "Networking/Networking.h"
#include "Util/JobSystem.h"
#include "ResourceManager.h"
#include "Platform/Window.h"
#include "Graphics/Renderer.h"
#include "Util/FileWatchdog.h"
#include "Physics/Physics.h"
#include "EngineSystems.h"

namespace Bat
{
	IApplication* CreateApplication( int argc, char* argv[], Renderer& gfx, Window& wnd );
}

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	using namespace Bat;

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

		Window wnd( { 50, 50 }, Renderer::InitialScreenWidth, Renderer::InitialScreenHeight, "Bat Engine", Renderer::FullScreen );
		BAT_TRACE( "Initialized window" );
		Renderer gfx( wnd );
		BAT_TRACE( "Initialized graphics" );

		FrameTimer ft;

		auto app = std::unique_ptr<IApplication>( CreateApplication( __argc, __argv, gfx, wnd ) );
		while( Window::ProcessMessagesForAllWindows() && wnd.IsOpen() )
		{
			float dt = ft.Mark();
			if( dt > 1.0f )
			{
				dt = 1.0f / 60.0f;
			}
			g_pGlobals->deltatime = dt;
			g_pGlobals->elapsed_time += dt;

			BAT_SERVICE_SYSTEMS( dt );

			app->OnUpdate( dt );

			gfx.BeginFrame();

			app->OnRender();
			g_Console.Draw("Bat Engine Console");

			gfx.EndFrame();

			wnd.input.SaveState();
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