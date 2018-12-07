#pragma once

#include "PCH.h"

namespace Bat
{
	class Texture;
	class Mesh;
	using MeshCollection = std::vector<Mesh>;
	class VertexShader;
	class PixelShader;

	template <typename T>
	using Resource = std::shared_ptr<T>;

	class ResourceManager
	{
	public:
		static Resource<Texture>        GetTexture( const std::string& filename );
		static Resource<MeshCollection> GetModelMeshes( const std::string& filename );
		static Resource<VertexShader>   GetVertexShader( const std::string& filename );
		static Resource<PixelShader>    GetPixelShader( const std::string& filename );

		static void CleanUp();
	};
}