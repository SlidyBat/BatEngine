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
		TEXTURE,
		RENDER_TEXTURE,
		DEPTH_STENCIL
	};

	struct RenderNode
	{
		std::string  name;
		NodeType     type;
		NodeDataType datatype;
	};
}