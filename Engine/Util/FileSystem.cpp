#include "PCH.h"
#include "FileSystem.h"

namespace Bat
{
	FileHandle_t FileSystem::Open( const char* filename, const char* mode )
	{
		BAT_LOG( "Opening %s (%s)", filename, mode );

		FILE* file;
		fopen_s( &file, filename, mode );
		return (FileHandle_t)file;
	}

	void FileSystem::Close( FileHandle_t handle )
	{
		fclose( (FILE*)handle );
	}

	size_t FileSystem::Read( FileHandle_t handle, char* out, size_t size )
	{
		return fread( out, 1, size, (FILE*)handle );
	}

	Future<size_t> FileSystem::ReadAsync( FileHandle_t handle, char* out, size_t size )
	{
		return std::async( &FileSystem::Read, this, handle, out, size );
	}

	size_t FileSystem::Write( FileHandle_t handle, const char* in, size_t size )
	{
		return fwrite( in, 1, size, (FILE*)handle );
	}

	Future<size_t> FileSystem::WriteAsync( FileHandle_t handle, const char* in, size_t size )
	{
		return std::async( &FileSystem::Write, this, handle, in, size );
	}

	void FileSystem::Flush( FileHandle_t handle )
	{
		fflush( (FILE*)handle );
	}

	void FileSystem::Seek( FileHandle_t handle, int off, SeekPos pos )
	{
		fseek( (FILE*)handle, off, (int)pos );
	}

	int FileSystem::Tell( FileHandle_t handle )
	{
		return ftell( (FILE*)handle );
	}

	bool FileSystem::Exists( const char* filename )
	{
		struct stat s;
		int rc = stat( filename, &s );

		return rc == 0;
	}

	size_t FileSystem::Size( FileHandle_t handle )
	{
		auto file = (FILE*)handle;

		long off = ftell( file );
		fseek( file, 0, SEEK_END );
		size_t size = (size_t)ftell( file );
		fseek( file, off, SEEK_SET );

		return size;
	}

	size_t FileSystem::Size( const char* filename )
	{
		struct stat s;
		int rc = stat( filename, &s );

		return rc == 0 ? (size_t)s.st_size : (size_t)-1;
	}

	std::string FileSystem::ReadAllText( const char* filename )
	{
		return ReadAll( filename, "r" );
	}

	Future<std::string> FileSystem::ReadAllTextAsync( const char* filename )
	{
		return std::async( &FileSystem::ReadAllText, this, filename );
	}

	std::string FileSystem::ReadAllBinary( const char* filename )
	{
		return ReadAll( filename, "rb" );
	}

	Future<std::string> FileSystem::ReadAllBinaryAsync( const char* filename )
	{
		return std::async( &FileSystem::ReadAllBinary, this, filename );
	}

	std::string FileSystem::ReadAll( const char* filename, const char* mode )
	{
		size_t filesize = Size( filename );

		std::string buf;
		buf.resize( filesize );

		FileHandle_t handle = Open( filename, mode );
		Read( handle, buf.data(), filesize );
		Close( handle );

		return buf;
	}
}
