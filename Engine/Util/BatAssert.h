#pragma once

#include "BatWinAPI.h"
#include "Common.h"
#include "StringLib.h"

#ifdef _DEBUG
#define ENGINE_ENABLE_ASSERTS
#endif

#ifdef ENGINE_ENABLE_ASSERTS
#define ASSERT( expr, ... ) if( expr ) {} \
	else \
	{\
		Detail::AssertMessage( #expr, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__ ); \
		BAT_DEBUG_BREAK(); \
	}
#else
#define ASSERT( expr, ... )
#endif

namespace Bat
{
	namespace Detail
	{
		template <typename... Args>
		inline void AssertMessage( const std::string& expr, const std::string& file, const std::string& function, int line, Args&&... args )
		{
			std::string msg = Bat::Format( std::forward<Args>( args )... );

			const std::string errormsg = "Failed assertion: '" + expr + "'\n" +
				msg + "\n\n" +
				"File: " + file + "\n" +
				"Function: " + function + "\n" +
				"Line: " + std::to_string( line );
			MessageBoxA( NULL, errormsg.c_str(), "Failed Assertion", MB_ICONWARNING );
		}
	}
}