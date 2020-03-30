#pragma once

#include "Thread.h"
#include <cstddef>
#include <cstdio>
#include <string>

namespace Bat
{
	using FileHandle_t = intptr_t;

	enum class SeekPos
	{
		START = 0,
		CURRENT = 1,
		END = 2
	};

	class FileSystem
	{
	public:
		static constexpr FileHandle_t INVALID_HANDLE = 0;
		static constexpr auto INVALID_SIZE = (size_t)-1;

		FileHandle_t Open( const char* filename, const char* mode );
		void Close( FileHandle_t handle );

		size_t Read( FileHandle_t handle, char* out, size_t size );
		Future<size_t> ReadAsync( FileHandle_t handle, char* out, size_t size );
		size_t Write( FileHandle_t handle, const char* in, size_t size );
		Future<size_t> WriteAsync( FileHandle_t handle, const char* in, size_t size );
		void Flush( FileHandle_t handle );

		void Seek( FileHandle_t handle, int off, SeekPos pos );
		int Tell( FileHandle_t handle ) const;

		bool Exists( const char* filename ) const;

		size_t Size( FileHandle_t handle ) const;
		size_t Size( const char* filename ) const;

		std::string ReadAllText( const char* filename );
		Future<std::string> ReadAllTextAsync( const char* filename );
		std::string ReadAllBinary( const char* filename );
		Future<std::string> ReadAllBinaryAsync( const char* filename );
	private:
		std::string ReadAll( const char* filename, const char* mode );
	};

	class File
	{
	public:
		File( FileSystem& filesystem, const char* filename, const char* mode );
		~File();
		File( const File& ) = delete;
		File& operator=( const File& ) = delete;
		File( File&& donor );
		File& operator=( File&& donor );
		
		void Close();

		size_t Read( char* out, size_t size );
		Future<size_t> ReadAsync( char* out, size_t size );
		size_t Write( const char* in, size_t size );
		Future<size_t> WriteAsync( const char* in, size_t size );
		void Flush();

		size_t Size() const;

		void Seek( int off, SeekPos pos );
		int Tell() const;
	private:
		FileHandle_t m_hFile = FileSystem::INVALID_HANDLE;
		FileSystem* m_pFileSystem = nullptr;
	};
}