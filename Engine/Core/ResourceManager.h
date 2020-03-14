#pragma once

#include <string>

namespace Bat
{
	class Texture;
	class Colour;
	class Mesh;
	class IVertexShader;
	class IPixelShader;
	struct ShaderMacro;

	template <typename T>
	using Resource = std::shared_ptr<T>;

	class ResourceManager
	{
	public:
		static Resource<Texture> GetTexture( const std::string& filename );
		static Resource<Mesh>    GetMesh( const std::string& filename );
		static IVertexShader*    GetVertexShader( const std::string& filename, const std::vector<ShaderMacro>& macros = {} );
		static IPixelShader*     GetPixelShader( const std::string& filename, const std::vector<ShaderMacro>& macros = {} );

		static void CleanUp();
	};
}