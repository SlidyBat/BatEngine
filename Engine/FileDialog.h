#pragma once

#include <optional>
#include <string>
#include <filesystem>

namespace Bat
{
	class FileDialog
	{
	public:
		static std::optional<std::filesystem::path> Open( const std::filesystem::path& initial_dir, const std::string& title = {} );
	};
}