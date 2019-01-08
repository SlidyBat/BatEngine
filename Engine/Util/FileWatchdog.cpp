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
			FileListenerHandle_t listen_handle;
			FileChangedCallback_t    callback;
		};
		std::string filename;
		std::vector<FileWatchListener> listeners;
		std::filesystem::file_time_type last_check_time;
	};

	static uint32_t listener_count;
	static std::vector<FileWatchCallbacks> watched_files;
	static HANDLE filechange_handle;

	static Mutex       filewatch_lock;
	static std::thread watcher_thread;

	static void WatchFiles()
	{
		while( true )
		{
			filewatch_lock.Lock();
			if( filechange_handle == INVALID_HANDLE_VALUE )
			{
				break;
			}
			filewatch_lock.Unlock();

			auto status = WaitForSingleObject( filechange_handle, INFINITE );

			switch( status )
			{
				case WAIT_OBJECT_0:
				{
					ScopedLock lock( filewatch_lock );
					for( auto& fwc : watched_files )
					{
						const auto& filename = fwc.filename;
						const std::filesystem::path path( filename );

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

							break;
						}
					}

					if( filechange_handle == INVALID_HANDLE_VALUE )
					{
						return;
					}

					BOOL res = FindNextChangeNotification( filechange_handle );
					ASSERT( res, "Failed to re-watch directory" );

					break;
				}
				case WAIT_TIMEOUT:
				{
					BAT_WARN( "File watchdog timed out" );

					break;
				}
				default:
				{
					ASSERT( false, "Unhandled wait status ({})", status );

					break;
				}
			}

			std::this_thread::yield();
		}
	}

	void FileWatchdog::Initialize()
	{
		auto currdir = std::filesystem::current_path();

		filechange_handle = FindFirstChangeNotification( currdir.string().c_str(),
			TRUE,
			FILE_NOTIFY_CHANGE_LAST_WRITE
		);

		ASSERT( filechange_handle != INVALID_HANDLE_VALUE, "Failed to create change handle. {}", GetLastWinErrorAsString() );

		watcher_thread = std::thread( WatchFiles );
	}

	void FileWatchdog::Shutdown()
	{
		filewatch_lock.Lock();
		FindCloseChangeNotification( filechange_handle );
		filechange_handle = INVALID_HANDLE_VALUE;
		filewatch_lock.Unlock();

		watcher_thread.join();
	}

	FileListenerHandle_t FileWatchdog::AddFileChangeListener( const std::string& filename, FileChangedCallback_t callback )
	{
		std::filesystem::path path( filename );
		if( !std::filesystem::exists( path ) )
		{
			ASSERT( false, "Path '{}' does not exist.", path.string() );
			return INVALID_LISTENER;
		}

		std::filesystem::path abspath = std::filesystem::absolute( path );
		std::string absfilename = abspath.string();

		ScopedLock lock( filewatch_lock );

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
	bool FileWatchdog::RemoveFileChangeListener( FileListenerHandle_t handle )
	{
		if( handle == INVALID_LISTENER )
		{
			return false;
		}

		for( size_t i = 0; i < watched_files.size(); i++ )
		{
			auto& listeners = watched_files[i].listeners;
			for( size_t j = 0; j < listeners.size(); i++ )
			{
				if( listeners[j].listen_handle == handle )
				{
					std::swap( listeners.back(), listeners[i] );
					listeners.pop_back();

					return true;
				}
			}
		}

		return false;
	}
}
