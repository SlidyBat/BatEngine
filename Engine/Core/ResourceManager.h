#pragma once

#include <string>

namespace Bat
{
	class Texture;
	class Colour;
	class Mesh;
	class IVertexShader;
	class IPixelShader;

	template <typename T>
	using Resource = std::shared_ptr<T>;

	class ResourceManager
	{
	public:
		static Resource<Texture>        GetTexture( const std::string& filename );
		static Resource<IVertexShader>   GetVertexShader( const std::string& filename );
		static Resource<IPixelShader>    GetPixelShader( const std::string& filename );

		static void CleanUp();
	};
}