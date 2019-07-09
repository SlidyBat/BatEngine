#include "PCH.h"

#include "EngineSystems.h"

namespace Bat
{
	std::vector<class EngineSystem*> _g_EngineSystems;

	EngineSystem::EngineSystem( const std::string& name, SystemInitFunc_t init, SystemDestroyFunc_t destroy, SystemServiceFunc_t service )
		:
		m_szName( name )
	{
		init();
		BAT_TRACE("Initialized %s", m_szName);
		m_DestroyFunction = std::move( destroy );
		m_ServiceFunction = std::move( service );
	}

	EngineSystem::~EngineSystem()
	{
		m_DestroyFunction();
		BAT_TRACE( "Shut down %s", m_szName );
	}
}