#pragma once

#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <spdlog/fmt/fmt.h>

namespace Bat
{
	std::wstring StringToWide( std::string_view str );
	std::string WideToString( std::wstring_view wstr );

	std::vector<std::string> SplitString( std::string_view str, const char delim = ' ' );
	std::string JoinStrings( const std::vector<std::string>& strings, const std::string& delim = " " );
	std::string JoinStrings( const std::vector<std::string_view>& strings, const std::string& delim = " " );

	std::string_view GetFileExtension( std::string_view filename );
	std::wstring_view GetFileExtension( std::wstring_view filename );

	bool IsWhitespace( const char c );
	bool IsLowercase( const char c );
	bool IsUppercase( const char c );
	bool IsNumeric( const char c );
	bool IsNumeric( std::string_view str );
	bool IsAlphabetic( const char c );
	bool IsAlphabetic( std::string_view str );
	bool IsAlphanumeric( const char c );
	bool IsAlphanumeric( std::string_view str );
	bool IsInteger( std::string_view str );
	bool IsFloat( std::string_view str );

	std::string Trim( std::string_view str );
	std::string ToLower( std::string str );
	std::string ToUpper( std::string str );

	template <typename... Args>
	std::string Format( const std::string& fmt, Args&&... args )
	{
		return fmt::format( fmt, std::forward<Args>(args)... );
	}
}