#pragma once

#include "PCH.h"
#include <d3d11.h>

namespace Bat
{
	enum class VertexAttribute
	{
		Invalid = -1,
		Position,
		Colour,
		Normal,
		UV,
		Tangent,
		Bitangent,
		BoneId,
		BoneWeight,
		TotalAttributes
	};
}