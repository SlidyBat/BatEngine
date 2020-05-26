#pragma once

#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/bundled/printf.h>

namespace Bat
{
	void StringToWide( const char* str, wchar_t* out_wstr, size_t size_bytes );
	std::wstring StringToWide( const std::string& str );
	void WideToString( const wchar_t* wstr, char* out_str, size_t size_bytes );
	std::string WideToString( const std::wstring& wstr );

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
	std::string Format( std::string_view fmt, Args&&... args )
	{
		return fmt::sprintf( fmt, std::forward<Args>( args )... );
	}
	template <typename... Args>
	int Scan( const std::string& buf, std::string_view fmt, Args&&... args )
	{
		return sscanf_s( buf.c_str(), fmt.data(), args... );
	}
}