#pragma once

#include <functional>
#include <string>

namespace Bat
{
	using FileChangedCallback_t = std::function<void( const std::string& filename )>;
	using FileWatchHandle_t = uint32_t;

	class FileWatchdog
	{
	public:
		static constexpr FileWatchHandle_t INVALID_WATCH_HANDLE = (uint32_t)-1;
	public:
		static void Initialize();
		static void Shutdown();

		static FileWatchHandle_t AddFileChangeListener( const std::string& filename, FileChangedCallback_t callback );
		static bool RemoveFileChangeListener( FileWatchHandle_t handle );
	};
}