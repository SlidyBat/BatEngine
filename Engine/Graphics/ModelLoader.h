#pragma once

#include "MathLib.h"
#include "Mesh.h"
#include <fstream>
#include <vector>

namespace Bat
{
	class ModelLoader
	{
	public:
		static std::vector<Mesh> LoadModel( const std::string& filename );
	};
}