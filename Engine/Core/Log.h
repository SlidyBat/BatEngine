#pragma once

//#define SPDLOG_USE_PRINTF
#include "StringLib.h"
#include "spdlog/spdlog.h"

#ifdef _DEBUG
#define BAT_DEBUG(...) Bat::Logger::GetLogger()->debug(Bat::Format(__VA_ARGS__))
#define BAT_TRACE(...) Bat::Logger::GetLogger()->trace(Bat::Format(__VA_ARGS__))
#define BAT_LOG(...)   Bat::Logger::GetLogger()->info(Bat::Format(__VA_ARGS__))
#define BAT_WARN(...)  Bat::Logger::GetLogger()->error(Bat::Format(__VA_ARGS__))
#define BAT_ERROR(...) Bat::Logger::GetLogger()->critical(Bat::Format(__VA_ARGS__))
#define BAT_ABORT(...) BAT_ERROR(Bat::Format(__VA_ARGS__)); ASSERT(false, Bat::Format(__VA_ARGS__));
#else
#define BAT_DEBUG(...)
#define BAT_TRACE(...)
#define BAT_LOG(...)   Bat::Logger::GetLogger()->info(Bat::Format(__VA_ARGS__))
#define BAT_WARN(...)  Bat::Logger::GetLogger()->error(Bat::Format(__VA_ARGS__))
#define BAT_ERROR(...) Bat::Logger::GetLogger()->critical(Bat::Format(__VA_ARGS__))
#define BAT_ABORT(...) BAT_ERROR(Bat::Format(__VA_ARGS__)); BAT_ASSERT(false, Bat::Format(__VA_ARGS__));
#endif

namespace Bat
{
	class Logger
	{
	public:
		static void Initialize();
		static void Shutdown() {}
		static std::shared_ptr<spdlog::logger> GetLogger();
	};
}