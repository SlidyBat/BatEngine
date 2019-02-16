#include "PCH.h"
#include "COMInitialize.h"

#include <objbase.h>

COMInitialize::COMInitialize()
{
	CoInitializeEx( NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE );
}

COMInitialize::~COMInitialize()
{
	CoUninitialize();
}
