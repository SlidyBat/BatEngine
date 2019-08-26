#include "PCH.h"
#include "FileWatchdog.h"

#include "BatWinAPI.h"
#include <unordered_map>
#include <vector>

#include "Mutex.h"
#include "Log.h"

namespace Bat
{
	struct FileWatchCallbacks
	{
		struct FileWatchListener
		{
			FileWatchHandle_t listen_handle;
			FileChangedCallback_t    callback;
		};
		std::string filename;
		std::vector<FileWatchListener> listeners;
		std::filesystem::file_time_type last_check_time;
	};

	static uint32_t listener_count;
	static std::vector<FileWatchCallbacks> watched_files;

	// Checks whether file is open by another process
	static bool FileIsAvailable( const std::filesystem::path& path )
	{
		HANDLE h = CreateFileW( path.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if( h != INVALID_HANDLE_VALUE )
		{
			CloseHandle( h );
			return true;
		}
		return false;
	}

	void FileWatchdog::Service( float deltatime )
	{
		for( auto& fwc : watched_files )
		{
			const auto& filename = fwc.filename;
			const std::filesystem::path path( filename );

			if( !FileIsAvailable( path ) )
			{
				continue;
			}

			// file was written to since we last checked
			const auto last_write_time = std::filesystem::last_write_time( path );
			if( last_write_time > fwc.last_check_time )
			{
				const auto& listeners = fwc.listeners;
				for( const auto& listener : listeners )
				{
					listener.callback( filename );
				}

				fwc.last_check_time = std::filesystem::file_time_type::clock::now();
			}
		}
	}

	void FileWatchdog::Initialize()
	{

	}

	void FileWatchdog::Shutdown()
	{

	}

	FileWatchHandle_t FileWatchdog::AddFileChangeListener( const std::string& filename, FileChangedCallback_t callback )
	{
		std::filesystem::path path( filename );
		if( !std::filesystem::exists( path ) )
		{
			ASSERT( false, "Path '%s' does not exist.", path.string() );
			return INVALID_WATCH_HANDLE;
		}

		std::filesystem::path abspath = std::filesystem::absolute( path );
		std::string absfilename = abspath.string();

		auto it = std::find_if( watched_files.begin(), watched_files.end(), [&absfilename]( const FileWatchCallbacks& fwc )
		{
			return fwc.filename == absfilename;
		} );

		FileWatchCallbacks::FileWatchListener listener;
		listener.listen_handle = listener_count++;
		listener.callback = std::move( callback );

		// new file to watch, add it
		if( it == watched_files.end() )
		{
			FileWatchCallbacks fwc;
			fwc.filename = absfilename;
			fwc.last_check_time = std::filesystem::file_time_type::clock::now();
			fwc.listeners.emplace_back( listener );
			watched_files.emplace_back( fwc );
		}
		// file is already being watched, just add the callback
		else
		{
			it->listeners.emplace_back( listener );
		}

		return listener.listen_handle;
	}
	bool FileWatchdog::RemoveFileChangeListener( FileWatchHandle_t handle )
	{
		if( handle == INVALID_WATCH_HANDLE )
		{
			return false;
		}

		for( size_t i = 0; i < watched_files.size(); i++ )
		{
			auto& listeners = watched_files[i].listeners;
			for( size_t j = 0; j < listeners.size(); j++ )
			{
				if( listeners[j].listen_handle == handle )
				{
					std::swap( listeners.back(), listeners[j] );
					listeners.pop_back();

					return true;
				}
			}
		}

		return false;
	}
}
