#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace Bat
{
	std::shared_ptr<spdlog::logger> g_pLogger;

	void Logger::Init()
	{
		AllocConsole();
		SetConsoleTitleA( "Bat Engine Console" );

		g_pLogger = spdlog::stdout_color_st( "console" );
		

#ifdef _DEBUG
		spdlog::set_level( spdlog::level::debug );
#else
		spdlog::set_level( spdlog::level::trace );
#endif
		spdlog::set_pattern( "[%I:%M:%S] %^%v%$" );
	}

	std::shared_ptr<spdlog::logger> Logger::GetLogger()
	{
		return g_pLogger;
	}
}