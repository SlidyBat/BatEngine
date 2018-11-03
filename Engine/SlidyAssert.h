#pragma once

#include "SlidyWin.h"
#include <string>

#ifdef _DEBUG
#define ENGINE_ENABLE_ASSERTS
#endif

#ifdef ENGINE_ENABLE_ASSERTS
#define ASSERT( expr, msg ) if( expr ) {} \
	else \
	{\
		Utils::AssertMessage( #expr, msg, __FILE__, __FUNCTION__, __LINE__ ); \
		__debugbreak(); \
	}
#else
#define ASSERT(expr)
#endif

namespace Utils
{
	inline void AssertMessage( const std::string& expr, const std::string& msg, const std::string& file, const std::string& function, int line )
	{
		const std::string errormsg = "Failed assertion: '" + expr + "'\n" +
			msg + "\n\n" +
			"File: " + file + "\n" +
			"Function: " + function + "\n" +
			"Line: " + std::to_string( line );
		MessageBoxA( NULL, errormsg.c_str(), "Failed Assertion", MB_ICONWARNING );
	}
}