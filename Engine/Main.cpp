#include "BatWinAPI.h"
#include "Window.h"
#include "Graphics.h"
#include "TestScene.h"
#include <string>

using namespace Bat;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow )
{
	try
	{
		Window wnd( { 50, 50 }, Graphics::ScreenWidth, Graphics::ScreenHeight, "Bat Engine", Graphics::FullScreen );
		Graphics gfx( wnd );
		//IGraphics::RegisterGraphics( gfx );

		IScene* pScene = new TestScene( wnd );
		while( wnd.ProcessMessage() )
		{
			pScene->OnUpdate();
			gfx.BeginFrame();
			pScene->OnRender();
			gfx.EndFrame();
		}
	}
	catch( const std::exception& e )
	{
		MessageBox( NULL, e.what(), "Error in window creation", MB_ICONWARNING );
	}

	return 0;
}