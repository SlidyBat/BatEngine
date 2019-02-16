#pragma once

#include <string>

namespace Bat
{
	// TODO allow quotes to pass in args with spaces
	class TokenStream
	{
	public:
		TokenStream( std::string_view str );

		std::string_view NextToken();
		bool operator>>( std::string_view& str );
	private:
		void SkipWhitespace();
	private:
		std::string_view m_szOriginalStr;
		int m_iCurrentPos = 0;
	};
}