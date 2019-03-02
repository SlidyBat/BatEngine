#include "PCH.h"
#include "ShaderManager.h"

namespace Bat
{
	static std::unordered_map<std::string, std::unique_ptr<IPipeline>> g_mapPipelines;

	IPipeline* ShaderManager::GetPipeline( const std::string& name )
	{
		auto it = g_mapPipelines.find( name );
		if( it == g_mapPipelines.end() )
		{
			return nullptr;
		}

		return it->second.get();
	}

	void ShaderManager::AddPipeline( const std::string& name, std::unique_ptr<IPipeline> pPipeline )
	{
		g_mapPipelines[name] = std::move( pPipeline );
	}
}
