#pragma once

#include "PCH.h"
#include "Mesh.h"

namespace Bat
{
	class ModelLoader
	{
	public:
		static std::vector<Mesh> LoadModel( const std::string& filename );
	};
}