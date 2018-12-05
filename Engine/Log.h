#pragma once

#include "spdlog/spdlog.h"

#ifdef _DEBUG
#define BAT_TRACE(...) Logger::GetLogger()->debug(__VA_ARGS__)
#define BAT_LOG(...) Logger::GetLogger()->trace(__VA_ARGS__)
#define BAT_WARN(...) Logger::GetLogger()->warn(__VA_ARGS__)
#define BAT_ERROR(...) Logger::GetLogger()->error(__VA_ARGS__)
#else
#define TRACE(...)
#define LOG(...) Logger::GetLogger()->trace(__VA_ARGS__)
#define WARN(...) Logger::GetLogger()->warn(__VA_ARGS__)
#define ERROR(...) Logger::GetLogger()->error(__VA_ARGS__)
#endif

namespace Bat
{
	class Logger
	{
	public:
		static void Init();
		static std::shared_ptr<spdlog::logger> GetLogger();
	};
}