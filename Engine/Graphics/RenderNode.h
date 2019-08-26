#pragma once

#include <string>

namespace Bat
{
	enum class NodeType
	{
		INVALID = -1,
		INPUT,
		OUTPUT
	};

	enum class NodeDataType
	{
		INVALID = -1,
#define RENDER_NODE_DATATYPE( type, name, capname ) \
		capname,
#include "RenderNodeDataTypes.def"
	};

	struct RenderNode
	{
		std::string  name;
		NodeType     type;
		NodeDataType datatype;
	};
}