#include "PCH.h"
#include "COMInitialize.h"

#include <objbase.h>

void COMInitialize::Initialize()
{
	CoInitializeEx( NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE );
}

void COMInitialize::Shutdown()
{
	CoUninitialize();
}
