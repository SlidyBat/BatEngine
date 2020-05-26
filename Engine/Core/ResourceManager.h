#pragma once

#include "Util/Thread.h"
#include <string>
#include <memory>

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
	template <typename T>
	using FutureResource = Future<Resource<T>>;

	class ResourceManager
	{
	public:
		static Resource<Texture>       GetTexture( const std::string& filename );
		static FutureResource<Texture> GetTextureAsync( const std::string& filename );
		static Resource<Mesh>          GetMesh( const std::string& filename );
		static FutureResource<Mesh>    GetMeshAsync( const std::string& filename );
		static IVertexShader*          GetVertexShader( const std::string& filename, const ShaderMacro* macros = nullptr, size_t num_macros = 0 );
		static Future<IVertexShader*>  GetVertexShaderAsync( const std::string& filename, const ShaderMacro* macros = nullptr, size_t num_macros = 0 );
		static IPixelShader*           GetPixelShader( const std::string& filename, const ShaderMacro* macros = nullptr, size_t num_macros = 0 );
		static Future<IPixelShader*>   GetPixelShaderAsync( const std::string& filename, const ShaderMacro* macros = nullptr, size_t num_macros = 0 );

		static void CleanUp();
	};
}