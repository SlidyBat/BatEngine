#include "PCH.h"
#include "RenderData.h"

namespace Bat
{
#define RENDER_NODE_DATATYPE( type, name, capname ) \
	type* RenderData::Get##name( const std::string& resource_name ) \
	{ \
		auto it = m_map##name.find( resource_name ); \
		if( it != m_map##name.end() ) \
		{ \
			return it->second; \
		} \
		return nullptr; \
	} \
	\
	void RenderData::Add##name( const std::string& resource_name, type* pResource ) \
	{ \
		m_map##name[resource_name] = pResource; \
	}
#include "RenderNodeDataTypes.def"
}
