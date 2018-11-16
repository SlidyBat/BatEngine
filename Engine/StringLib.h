#pragma once

#include <string>

namespace Bat
{
	inline std::wstring StringToWide( const std::string& str )
	{
		return std::wstring( str.begin(), str.end() );
	}
}