#include "Reflect.h"

#include <string>

namespace Bat
{
#define BAT_REFLECT_PRIMITIVE( prim ) \
	template <> \
	TypeDescriptor GetTypeDescriptorImpl<prim>() \
	{ \
		TypeDescriptor desc; \
		desc.name = #prim; \
		desc.size = sizeof( prim ); \
		desc.num_members = 0; \
		desc.members = nullptr; \
		return desc; \
	}

	BAT_REFLECT_PRIMITIVE( int );
	BAT_REFLECT_PRIMITIVE( short );
	BAT_REFLECT_PRIMITIVE( bool );
	BAT_REFLECT_PRIMITIVE( float );
	BAT_REFLECT_PRIMITIVE( double );
	BAT_REFLECT_PRIMITIVE( std::string );
}