#include "ObjLoader.h"

#include "BatAssert.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Bat
{
	std::vector<Mesh> ObjLoader::LoadMeshes( std::istream & objfile )
	{
		return std::vector<Mesh>();
	}
}
