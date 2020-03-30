#include "PCH.h"
#include "Globals.h"

#include "FileSystem.h"

namespace Bat
{
	static GlobalValues g_Globals;
	GlobalValues* g_pGlobals = &g_Globals;

	static FileSystem g_Filesystem;
	FileSystem* filesystem = &g_Filesystem;
}