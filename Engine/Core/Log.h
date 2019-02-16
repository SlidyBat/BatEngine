#pragma once

#include "spdlog/spdlog.h"

#ifdef _DEBUG
#define BAT_DEBUG(...) Bat::Logger::GetLogger()->debug(__VA_ARGS__)
#define BAT_TRACE(...) Bat::Logger::GetLogger()->trace(__VA_ARGS__)
#define BAT_LOG(...)   Bat::Logger::GetLogger()->info(__VA_ARGS__)
#define BAT_WARN(...)  Bat::Logger::GetLogger()->error(__VA_ARGS__)
#define BAT_ERROR(...) Bat::Logger::GetLogger()->critical(__VA_ARGS__)
#else
#define BAT_DEBUG(...)
#define BAT_TRACE(...)
#define BAT_LOG(...)   Bat::Logger::GetLogger()->info(__VA_ARGS__)
#define BAT_WARN(...)  Bat::Logger::GetLogger()->error(__VA_ARGS__)
#define BAT_ERROR(...) Bat::Logger::GetLogger()->critical(__VA_ARGS__)
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