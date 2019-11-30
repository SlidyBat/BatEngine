#include "FileDialog.h"

#define FULL_WINTARD
#include "BatWinAPI.h"
#include <commdlg.h>

namespace Bat
{
	std::optional<std::filesystem::path> FileDialog::Open( const std::filesystem::path& initial_dir, const std::string& title )
	{
		char filename[MAX_PATH];
		filename[0] = '\0';

		OPENFILENAME of = {};
		of.lStructSize = sizeof( of );
		of.lpstrFile = filename;
		of.nMaxFile = sizeof( filename );
		of.lpstrInitialDir = std::filesystem::absolute( initial_dir ).string().c_str();
		if( !title.empty() )
		{
			of.lpstrTitle = title.c_str();
		}
		of.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if( GetOpenFileName( &of ) != 0 )
		{
			return filename;
		}

		return {};
	}
}