#pragma once

#include "MathLib.h"
#include "Mesh.h"
#include <fstream>
#include <vector>

namespace Bat
{
	class ObjLoader
	{
	public:
		static std::vector<Mesh> LoadMeshes( std::istream& objfile );
		static std::vector<Mesh> LoadMeshes( const std::string& filename )
		{
			return LoadMeshes( std::ifstream( filename, std::ios::binary ) );
		}
	};
}