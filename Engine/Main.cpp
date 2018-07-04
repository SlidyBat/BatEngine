#include "SlidyWin.h"
#include "Window.h"
#include "Graphics.h"
#include "Game.h"
#include <string>

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow )
{
	try
	{
		Window wnd( 50, 50, Graphics::ScreenWidth, Graphics::ScreenHeight, "Slidy Engine", Graphics::FullScreen );

		try
		{
			Game game( wnd );
			while( wnd.ProcessMessage() )
			{
				game.Run();
			}
		}
		catch( const std::exception& e )
		{
			wnd.ShowMessageBox( "Runtime error", e.what(), MB_OK );
		}
	}
	catch( const std::exception& e )
	{
		MessageBox( NULL, e.what(), "Error in window creation", MB_OK );
	}

	return 0;
}