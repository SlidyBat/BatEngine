#include "BatWinAPI.h"
#include "COMInitialize.h"
#include "Window.h"
#include "Graphics.h"
#include "MarioTestScene.h"
#include "ColourTestScene.h"
#include "ModelTestScene.h"
#include "CarTestScene.h"
#include "COMException.h"
#include <string>
#include "FrameTimer.h"
#include "Globals.h"
#include "StringLib.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "Log.h"
#include "JobSystem.h"

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

		auto pScene = std::make_unique<ModelTestScene>( wnd );
		while( wnd.ProcessMessage() )
		{
			static int fpsCounter = 0;
			static float elapsedTime = 0.0f;
			static std::string fpsString = "FPS: 0";

			float dt = ft.Mark();
			g_pGlobals->deltatime = dt;
			elapsedTime += dt;
			fpsCounter += 1;
			if( elapsedTime > 1.0f )
			{
				fpsString = "FPS: " + std::to_string( fpsCounter );
				fpsCounter = 0;
				elapsedTime -= 1.0f;
			}
			g_pGlobals->elapsed_time += dt;

			pScene->OnUpdate( dt );

			gfx.BeginFrame();
			
			pScene->OnRender();

			gfx.DrawText( Bat::StringToWide( fpsString ).c_str(), DirectX::XMFLOAT2{ 15.0f, 15.0f } );

			gfx.EndFrame();
		}
	}
	catch( const std::exception& e )
	{
		MessageBox( NULL, e.what(), "Error", MB_ICONWARNING | MB_OK );
	}

	return 0;
}