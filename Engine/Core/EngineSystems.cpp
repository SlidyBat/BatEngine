#include "PCH.h"

#include "EngineSystems.h"

namespace Bat
{
	EngineSystem::EngineSystem( const std::string& name, SystemInitFunc_t init, SystemDestroyFunc_t destroy )
		:
		m_szName( name )
	{
		init();
		BAT_TRACE("Initialized %s", m_szName);
		m_DestroyFunction = std::move( destroy );
	}

	EngineSystem::~EngineSystem()
	{
		m_DestroyFunction();
		BAT_TRACE( "Shut down %s", m_szName );
	}
}