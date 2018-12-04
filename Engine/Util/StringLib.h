#pragma once

#include <string>
#include <string_view>
#include <sstream>
#include <vector>

namespace Bat
{
	inline std::wstring StringToWide( std::string_view str )
	{
		return std::wstring( str.begin(), str.end() );
	}

	inline std::string WideToString( std::wstring_view wstr )
	{
		return std::string( wstr.begin(), wstr.end() );
	}

	inline std::vector<std::string> SplitString( std::string_view str, const char delim = ' ' )
	{
		std::vector<std::string> elems;
		size_t start = 0;
		size_t count = 0;
		for( size_t i = 0; i < str.size(); i++ )
		{
			if( str[i] == delim )
			{
				elems.emplace_back( str.substr( start, count ) );
				start = i + 1;
				count = 0;
			}
			else
			{
				count++;
			}
		}

		elems.emplace_back( str.substr( start, count ) );

		return elems;
	}

	inline std::string GetFileExtension( std::string_view filename )
	{
		size_t off = filename.find_last_of( '.' );
		if( off == std::string::npos )
		{
			return {};
		}

		return std::string( filename.substr( off + 1 ) );
	}
	inline std::wstring GetFileExtension( std::wstring_view filename )
	{
		size_t off = filename.find_last_of( '.' );
		if( off == std::string::npos )
		{
			return {};
		}

		return std::wstring( filename.substr( off + 1 ) );
	}
}