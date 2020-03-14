#include "PCH.h"

#include "ResourceManager.h"

#include "Texture.h"
#include "Colour.h"
#include "Mesh.h"
#include "SceneLoader.h"
#include "IGPUDevice.h"

namespace Bat
{
	template<typename T>
	using ResourceMap = std::unordered_map<std::string, Resource<T>>;
	template<typename T>
	using UniqueResourceMap = std::unordered_map<std::string, std::unique_ptr<T>>;

	static ResourceMap<Texture>             g_mapTextures;
	static ResourceMap<Mesh>                g_mapMeshes;
	static UniqueResourceMap<IVertexShader> g_mapVShaders;
	static UniqueResourceMap<IPixelShader>  g_mapPShaders;

	Resource<Texture> ResourceManager::GetTexture( const std::string& filename )
	{
		auto it = g_mapTextures.find( filename );
		if( it == g_mapTextures.end() )
		{
			auto pResource = std::make_shared<Texture>( StringToWide( filename ) );
			g_mapTextures[filename] = pResource;
			return pResource;
		}

		return it->second;
	}

	Resource<Mesh> ResourceManager::GetMesh( const std::string& filename )
	{
		auto it = g_mapMeshes.find( filename );
		if( it == g_mapMeshes.end() )
		{
			SceneLoader loader;
			std::shared_ptr<Mesh> mesh = loader.LoadMesh( filename );
			g_mapMeshes[filename] = mesh;
			return mesh;
		}

		return it->second;
	}

	static std::string GetShaderHashName( const std::string& filename, const std::vector<ShaderMacro>& macros )
	{
		std::string name = filename;
		for( const auto& macro : macros )
		{
			name += macro.name;
			name += macro.value;
		}
		return name;
	}
	
	IVertexShader* ResourceManager::GetVertexShader( const std::string& filename, const std::vector<ShaderMacro>& macros )
	{
		std::string name = GetShaderHashName( filename, macros );
		auto it = g_mapVShaders.find( name );
		if( it == g_mapVShaders.end() )
		{
			auto pResource = gpu->CreateVertexShader( filename, macros );
			g_mapVShaders[name] = std::unique_ptr<IVertexShader>( pResource );
			return pResource;
		}

		return it->second.get();
	}

	IPixelShader* ResourceManager::GetPixelShader( const std::string& filename, const std::vector<ShaderMacro>& macros )
	{
		std::string name = GetShaderHashName( filename, macros );
		auto it = g_mapPShaders.find( name );
		if( it == g_mapPShaders.end() )
		{
			auto pResource = gpu->CreatePixelShader( filename, macros );
			g_mapPShaders[name] = std::unique_ptr<IPixelShader>( pResource );
			return pResource;
		}

		return it->second.get();
	}

	template <typename T>
	static void CleanUpResources( std::unordered_map<std::string, std::shared_ptr<T>>& resource_map )
	{
		for( auto i = resource_map.begin(); i != resource_map.end(); )
		{
			if( i->second.use_count() == 1 )
			{
				i = resource_map.erase( i );
			}
			else
			{
				++i;
			}
		}
	}

	void ResourceManager::CleanUp()
	{
		CleanUpResources( g_mapTextures );

		g_mapVShaders.clear();
		g_mapPShaders.clear();
	}
}