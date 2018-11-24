#include "COMInitialize.h"

#include <objbase.h>

COMInitialize::COMInitialize()
{
	CoInitializeEx( NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE );
}

COMInitialize::~COMInitialize()
{
	CoUninitialize();
}
