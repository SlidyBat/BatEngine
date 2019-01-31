#include "PCH.h"
#include "TokenStream.h"

#include "StringLib.h"

namespace Bat
{
	TokenStream::TokenStream( std::string_view str )
		:
		m_szOriginalStr( str )
	{}

	std::string_view TokenStream::NextToken()
	{
		SkipWhitespace();
		
		const int start = m_iCurrentPos;

		while( m_iCurrentPos < m_szOriginalStr.size() && !Bat::IsWhitespace( m_szOriginalStr[m_iCurrentPos] ) )
		{
			m_iCurrentPos++;
		}

		if( m_iCurrentPos == start )
		{
			return {};
		}

		return m_szOriginalStr.substr( start, m_iCurrentPos - start );
	}

	bool TokenStream::operator>>( std::string_view& str )
	{
		str = NextToken();
		return !str.empty();
	}

	void TokenStream::SkipWhitespace()
	{
		while( m_iCurrentPos < m_szOriginalStr.size() && Bat::IsWhitespace( m_szOriginalStr[m_iCurrentPos] ) )
		{
			m_iCurrentPos++;
		}
	}
}