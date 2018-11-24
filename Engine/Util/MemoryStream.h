#pragma once

#include <vector>

class MemoryStream
{
public:
	enum class SeekPosition
	{
		Start,
		End,
		Current
	};
public:
	MemoryStream() = default;
	MemoryStream( const char* data, size_t size );
	MemoryStream( std::vector<char> data );

	void Seek( SeekPosition where );
	void Seek( size_t pos, SeekPosition dir );
	size_t Tell();

	char ReadByte();
	void ReadBytes( char* pBytes, const size_t size );
	template <typename T>
	T Read()
	{
		const T val;
		const char* pStart = reinterpret_cast<const char*>(&val);
		const size_t size = sizeof( T );
		ReadBytes( pStart, size );

		return val;
	}

	void WriteByte( const char byte );
	void WriteBytes( const char* pBytes, const size_t size );
	template <typename T>
	void Write( const T& val )
	{
		const char* pStart = reinterpret_cast<const char*>(&val);
		const size_t size = sizeof( T );
		WriteBytes( pStart, size );
	}

	bool EndOfStream() const;
	size_t Size() const;
	const char* Base() const;
	char* Base();

	void Clear();

	static MemoryStream FromStream( std::istream& stream );
	static MemoryStream FromFile( const std::string& filename );
	static MemoryStream FromFile( const std::wstring& filename );
private:
	size_t m_iCurrentByte = 0;
	std::vector<char> m_Bytes;
};