#include "SlidyWin.h"
#include "System.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow )
{
	System system;
	if( system.Initialize() )
	{
		system.Run();
	}

	return 0;
}