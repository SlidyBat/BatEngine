#include "SlidyWin.h"
#include "SystemClass.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow )
{
	SystemClass system;
	if( system.Initialize() )
	{
		system.Run();
	}

	return 0;
}