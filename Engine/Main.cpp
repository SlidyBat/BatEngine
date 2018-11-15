#include "BatWinAPI.h"
#include "Window.h"
#include "Graphics.h"
#include "MarioTestScene.h"
#include "ColourTestScene.h"
#include "ModelTestScene.h"
#include "COMException.h"
#include <string>
#include "FrameTimer.h"
#include "Globals.h"

using namespace Bat;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow )
{
	try
	{
		COM_ERROR_IF_FAILED( CoInitialize( NULL ) );

		Window wnd( { 50, 50 }, Graphics::ScreenWidth, Graphics::ScreenHeight, "Bat Engine", Graphics::FullScreen );
		Graphics gfx( wnd );
		//IGraphics::RegisterGraphics( gfx );

		FrameTimer ft;

		IScene* pScene = new ModelTestScene( wnd );
		while( wnd.ProcessMessage() )
		{
			float dt = ft.Mark();
			g_pGlobals->elapsed_time += dt;
			pScene->OnUpdate( dt );
			gfx.BeginFrame();
			pScene->OnRender();
			gfx.EndFrame();
		}
		delete pScene;
	}
	catch( const std::exception& e )
	{
		MessageBox( NULL, e.what(), "Error", MB_ICONWARNING );
	}

	return 0;
}