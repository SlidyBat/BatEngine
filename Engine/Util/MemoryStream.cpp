#include "MemoryStream.h"

#include <cassert>
#include <fstream>

MemoryStream::MemoryStream( const char* data, size_t size )
{
	m_Bytes.reserve( size );
	for( size_t i = 0; i < size; i++ )
	{
		m_Bytes.emplace_back( data[i] );
	}
}

MemoryStream::MemoryStream( std::vector<char> data )
	:
	m_Bytes( std::move( data ) )
{
}

void MemoryStream::Seek( SeekPosition where )
{
	if( where == SeekPosition::Start )
	{
		m_iCurrentByte = 0;
	}
	else if( where == SeekPosition::End )
	{
		m_iCurrentByte = m_Bytes.size();
	}
}

void MemoryStream::Seek( size_t pos, SeekPosition dir )
{
	switch( dir )
	{
	case SeekPosition::Start:
		m_iCurrentByte = pos;
		break;
	case SeekPosition::End:
		assert( pos < m_Bytes.size() );
		m_iCurrentByte = m_Bytes.size() - pos;
		break;
	case SeekPosition::Current:
		m_iCurrentByte += pos;
		break;
	}

	assert( false );
}

size_t MemoryStream::Tell()
{
	return m_iCurrentByte;
}

char MemoryStream::ReadByte()
{
	if( EndOfStream() )
	{
		return 0;
	}

	return m_Bytes[m_iCurrentByte++];
}

void MemoryStream::ReadBytes( char* pBytes, const size_t size )
{
	if( EndOfStream() )
	{
		return;
	}

	size_t newsize = size;
	if( m_iCurrentByte + size >= m_Bytes.size() )
	{
		newsize = m_Bytes.size() - m_iCurrentByte;
	}

	for( size_t i = 0; i < newsize; i++ )
	{
		pBytes[i] = m_Bytes[m_iCurrentByte++];
	}
}

void MemoryStream::WriteByte( const char byte )
{
	if( EndOfStream() )
	{
		m_Bytes.emplace_back( byte );
		m_iCurrentByte++;
	}

	m_Bytes[m_iCurrentByte++] = byte;
}

void MemoryStream::WriteBytes( const char* pBytes, const size_t size )
{
	if( EndOfStream() )
	{
		m_Bytes.reserve( m_Bytes.size() + size );
		for( size_t i = 0; i < size; i++ )
		{
			m_Bytes.emplace_back( pBytes[i] );
		}

		m_iCurrentByte += size;
	}
	else if( m_iCurrentByte + size > m_Bytes.size() )
	{
		const size_t total_size = m_Bytes.size();
		const size_t needed = (m_iCurrentByte + size) - total_size;
		m_Bytes.reserve( total_size + needed );

		for( size_t i = m_iCurrentByte; i < total_size; i++ )
		{
			m_Bytes[m_iCurrentByte + i] = pBytes[i];
		}

		const size_t off = total_size - m_iCurrentByte;
		for( size_t i = 0; i < needed; i++ )
		{
			m_Bytes.emplace_back( pBytes[off + i] );
		}
		
		m_iCurrentByte += size;
	}
	else
	{
		for( size_t i = 0; i < size; i++ )
		{
			m_Bytes[m_iCurrentByte++] = pBytes[i];
		}
	}
}

bool MemoryStream::EndOfStream() const
{
	return m_iCurrentByte >= m_Bytes.size();
}

size_t MemoryStream::Size() const
{
	return m_Bytes.size();
}

const char* MemoryStream::Base() const
{
	return m_Bytes.data();
}

char* MemoryStream::Base()
{
	return m_Bytes.data();
}

void MemoryStream::Clear()
{
	m_Bytes.clear();
	m_iCurrentByte = 0;
}

MemoryStream MemoryStream::FromStream( std::istream& stream )
{
	stream.seekg( 0, std::ios::end );
	size_t size = (size_t)stream.tellg();
	stream.seekg( 0, std::ios::beg );

	auto bytes = std::make_unique<char[]>( size );
	stream.read( bytes.get(), size );

	return MemoryStream( bytes.get(), size );
}

MemoryStream MemoryStream::FromFile( const std::string& filename )
{
	return FromStream( std::ifstream( filename, std::ios::binary ) );
}

MemoryStream MemoryStream::FromFile( const std::wstring & filename )
{
	return FromStream( std::ifstream( filename, std::ios::binary ) );
}
