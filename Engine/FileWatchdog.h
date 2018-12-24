#pragma once

#include <functional>
#include <string>

namespace Bat
{
	using FileChangedCallback_t = std::function<void( const std::string& filename )>;

	class FileWatchdog
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void AddFileChangeListener( const std::string& filename, FileChangedCallback_t callback );
	};
}