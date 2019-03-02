#pragma once

#include <cstdlib>

#define BYTESWAP16( s ) _byteswap_ushort( s )
#define BYTESWAP32( l ) _byteswap_ulong( l )
#define BYTESWAP64( q ) _byteswa_uint64( q )

namespace Bat
{
	inline bool IsMachineBigEndian()
	{
		short nIsBigEndian = 1;

		// if we are big endian, the first byte will be a 0, if little endian, it will be a one.
		return (bool)(0 ==  *(char *)&nIsBigEndian );
	}
}