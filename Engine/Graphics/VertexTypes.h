#pragma once

#include <d3d11.h>

namespace Bat
{
	enum class VertexAttribute
	{
		Invalid = -1,
		VertexId,
		Position,
		Colour,
		Normal,
		UV,
		Tangent,
		Bitangent,
		BoneId,
		BoneWeight,
		InstanceData,
		TotalAttributes
	};
}