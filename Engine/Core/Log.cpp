#include "PCH.h"
#include "Log.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Bat
{
	static std::shared_ptr<spdlog::logger> g_pLogger;

	static spdlog::level::level_enum Bat2SpdlogSeverity(int severity)
	{
		switch( severity )
		{
			case Logger::LOG_SEVERITY_DEBUG:
				return spdlog::level::debug;
			case Logger::LOG_SEVERITY_TRACE:
				return spdlog::level::trace;
			case Logger::LOG_SEVERITY_INFO:
				return spdlog::level::info;
			case Logger::LOG_SEVERITY_WARN:
				return spdlog::level::err;
			case Logger::LOG_SEVERITY_ERROR:
				return spdlog::level::critical;
			default:
				ASSERT( false, "Unhandled log severity" );
				return spdlog::level::info;
		}
	}

	void Logger::Initialize()
	{
		AllocConsole();
		SetConsoleTitleA( "Bat Engine Console" );

		g_pLogger = spdlog::stdout_color_mt( "console" );
		
		spdlog::set_pattern( "[%I:%M:%S] - [%^%l%$] %v" );
	}

	void Logger::Shutdown()
	{
		
	}

	void Logger::Log( const Logger::Severity severity, const std::string& msg, const std::string& file, const std::string& func, const int line )
	{
		g_pLogger->log( Bat2SpdlogSeverity( severity ), msg );
	}

	void Logger::Debug( const std::string& msg, const std::string& file, const std::string& func, const int line )
	{
		Log( LOG_SEVERITY_DEBUG, msg, file, func, line );
	}

	void Logger::Trace( const std::string& msg, const std::string& file, const std::string& func, const int line )
	{
		Log( LOG_SEVERITY_TRACE, msg, file, func, line );
	}

	void Logger::Info( const std::string& msg, const std::string& file, const std::string& func, const int line )
	{
		Log( LOG_SEVERITY_INFO, msg, file, func, line );
	}

	void Logger::Warn( const std::string& msg, const std::string& file, const std::string& func, const int line )
	{
		Log( LOG_SEVERITY_WARN, msg, file, func, line );
	}

	void Logger::Error( const std::string& msg , const std::string& file, const std::string& func, const int line)
	{
		Log( LOG_SEVERITY_ERROR, msg, file, func, line );
	}

	void Logger::Abort( const std::string& msg, const std::string& file, const std::string& func, const int line )
	{
		Error( msg, file, func, line );
		Flush(); // Make sure message makes it to console

		// Trigger an assert at the original location
		Bat::Detail::AssertMessage( false, file, func, line, "%s", msg );

		// Exit application
		throw std::runtime_error( Bat::Format( "Fatal Error!\n%s", msg ) );
	}

	void Logger::Flush()
	{
		g_pLogger->flush();
	}
}