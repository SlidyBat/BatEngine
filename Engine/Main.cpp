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

		IScene* scene = new TestScene( wnd, gfx );
		while( wnd.ProcessMessage() )
		{
			scene->OnUpdate();
			gfx.BeginFrame();
			scene->OnRender();
			gfx.EndFrame();
		}
	}
	catch( const std::exception& e )
	{
		MessageBox( NULL, e.what(), "Error in window creation", MB_ICONWARNING );
	}

	return 0;
}