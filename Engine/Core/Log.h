#pragma once

//#define SPDLOG_USE_PRINTF
#include "Util/StringLib.h"

#ifdef _DEBUG
#define BAT_DEBUG(...) Bat::Logger::Debug(Bat::Format(__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)
#define BAT_TRACE(...) Bat::Logger::Trace(Bat::Format(__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)
#define BAT_LOG(...)   Bat::Logger::Info(Bat::Format(__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)
#define BAT_WARN(...)  Bat::Logger::Warn(Bat::Format(__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)
#define BAT_ERROR(...) Bat::Logger::Error(Bat::Format(__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)
#define BAT_ABORT(...) do { BAT_DEBUG_BREAK(); Bat::Logger::Abort(Bat::Format(__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__); } while( false )
#else
#define BAT_DEBUG(...)
#define BAT_TRACE(...)
#define BAT_LOG(...)   Bat::Logger::Info(Bat::Format(__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)
#define BAT_WARN(...)  Bat::Logger::Warn(Bat::Format(__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)
#define BAT_ERROR(...) Bat::Logger::Error(Bat::Format(__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)
#define BAT_ABORT(...) Bat::Logger::Abort(Bat::Format(__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)
#endif

namespace Bat
{
	class Logger
	{
	public:
		enum Severity
		{
			LOG_SEVERITY_DEBUG = 0, // Only enabled on debug builds
			LOG_SEVERITY_TRACE,
			LOG_SEVERITY_INFO,
			LOG_SEVERITY_WARN,
			LOG_SEVERITY_ERROR
		};

		static void Initialize();
		static void Shutdown();

		static void Log( const Logger::Severity severity, const std::string& msg, const std::string& file, const std::string& func, const int line );

		static void Debug( const std::string& msg, const std::string& file, const std::string& func, const int line );
		static void Trace( const std::string& msg, const std::string& file, const std::string& func, const int line );
		static void Info( const std::string& msg, const std::string& file, const std::string& func, const int line );
		static void Warn( const std::string& msg, const std::string& file, const std::string& func, const int line );
		static void Error( const std::string& msg, const std::string& file, const std::string& func, const int line );
		static void Abort( const std::string& msg, const std::string& file, const std::string& func, const int line );

		static void Flush();
	};
}