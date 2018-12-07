#include "ResourceManager.h"

#include <unordered_map>
#include "StringLib.h"
#include "ModelLoader.h"

#include "Texture.h"
#include "Mesh.h"
#include "VertexShader.h"
#include "PixelShader.h"

namespace Bat
{
	template<typename T>
	using ResourceMap = std::unordered_map<std::string, std::shared_ptr<T>>;

	static ResourceMap<Texture>        g_mapTextures;
	static ResourceMap<MeshCollection> g_mapMeshes;
	static ResourceMap<VertexShader>   g_mapVShaders;
	static ResourceMap<PixelShader>    g_mapPShaders;

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

	Resource<MeshCollection> ResourceManager::GetModelMeshes( const std::string& filename )
	{
		auto it = g_mapMeshes.find( filename );
		if( it == g_mapMeshes.end() )
		{
			auto pResource = std::make_shared<MeshCollection>( std::move( ModelLoader::LoadModel( filename ) ) );
			g_mapMeshes[filename] = pResource;
			return pResource;
		}

		return it->second;
	}
	
	Resource<VertexShader> ResourceManager::GetVertexShader( const std::string& filename )
	{
		auto it = g_mapVShaders.find( filename );
		if( it == g_mapVShaders.end() )
		{
			auto pResource = std::make_shared<VertexShader>( StringToWide( filename ) );
			g_mapVShaders[filename] = pResource;
			return pResource;
		}

		return it->second;
	}

	Resource<PixelShader> ResourceManager::GetPixelShader( const std::string& filename )
	{
		auto it = g_mapPShaders.find( filename );
		if( it == g_mapPShaders.end() )
		{
			auto pResource = std::make_shared<PixelShader>( StringToWide( filename ) );
			g_mapPShaders[filename] = pResource;
			return pResource;
		}

		return it->second;
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
		CleanUpResources( g_mapMeshes );
		CleanUpResources( g_mapVShaders );
		CleanUpResources( g_mapPShaders );
	}
}