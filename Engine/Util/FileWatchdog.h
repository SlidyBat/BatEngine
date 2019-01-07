#pragma once

#include <functional>
#include <string>

namespace Bat
{
	using FileChangedCallback_t = std::function<void( const std::string& filename )>;
	using FileListenerHandle_t = uint32_t;

	class FileWatchdog
	{
	public:
		static constexpr FileListenerHandle_t INVALID_LISTENER = (uint32_t)-1;
	public:
		static void Initialize();
		static void Shutdown();

		static FileListenerHandle_t AddFileChangeListener( const std::string& filename, FileChangedCallback_t callback );
		static bool RemoveFileChangeListener( FileListenerHandle_t handle );
	};
}