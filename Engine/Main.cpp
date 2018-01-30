#include "SlidyWin.h"
#include "System.h"
#include <string>

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow )
{
	try
	{
		System system;
		system.Run();
	}
	catch( const std::exception& e )
	{
		MessageBox( NULL, e.what(), "Error", MB_OK );
	}

	return 0;
}