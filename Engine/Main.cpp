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

		{
			EntityManager manager;

			Entity e = manager.CreateEntity();

			manager.AddComponent<PositionComponent>( e, 1.0f, 2.0f, 3.0f );
			auto& position = manager.GetComponent<PositionComponent>( e );
			BAT_LOG( "Position component: [{}, {}, {}]", position.x, position.y, position.z );

			manager.DestroyEntity( e );
		}

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


			gfx.EndFrame();
		}
	}
	catch( const std::exception& e )
	{
		MessageBox( NULL, e.what(), "Error", MB_ICONWARNING | MB_OK );
	}

	FileWatchdog::Shutdown();
	ResourceManager::CleanUp();

	return 0;
}