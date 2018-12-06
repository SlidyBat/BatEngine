#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace Bat
{
	std::shared_ptr<spdlog::logger> g_pLogger;

	void Logger::Init()
	{
		AllocConsole();
		SetConsoleTitleA( "Bat Engine Console" );

		g_pLogger = spdlog::stdout_color_mt( "console" );
		
		spdlog::set_pattern( "[%I:%M:%S] - [%^%l%$] %v" );
	}

	std::shared_ptr<spdlog::logger> Logger::GetLogger()
	{
		return g_pLogger;
	}
}