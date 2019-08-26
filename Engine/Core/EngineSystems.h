#pragma once

#include <functional>

// Helper macro that creates an RAII class to auto-init/shutdown a system
// The pre-requisite is that the system class contains static Initialize/Shutdown functions
#define BAT_INIT_SYSTEM(name) EngineSystem _Bat_##name##_System = EngineSystem( #name, name::Initialize, name::Shutdown )
// Helper macro that creates an RAII class to auto-init/shutdown a system, as well as auto-call a service function every frame
// The pre-requisite is that the system class contains static Initialize/Shutdown/Service functions
#define BAT_INIT_PER_FRAME_SYSTEM(name) EngineSystem _Bat_##name##_System = EngineSystem( #name, name::Initialize, name::Shutdown, name::Service ); \
	Bat::_g_EngineSystems.emplace_back( &_Bat_##name##_System )

#define BAT_SERVICE_SYSTEMS( deltatime ) for( EngineSystem* system : _g_EngineSystems ) { system->Service( (deltatime) ); }

namespace Bat
{
	extern std::vector<class EngineSystem*> _g_EngineSystems;

	using SystemInitFunc_t = std::function<void()>;
	using SystemDestroyFunc_t = std::function<void()>;
	using SystemServiceFunc_t = std::function<void(float)>;

	class EngineSystem
	{
	public:
		EngineSystem( const std::string& name, SystemInitFunc_t init, SystemDestroyFunc_t destroy, SystemServiceFunc_t service = [](float) {} );
		~EngineSystem();

		void Service( float deltatime ) { m_ServiceFunction( deltatime ); }
	private:
		std::string m_szName;
		SystemDestroyFunc_t m_DestroyFunction;
		SystemServiceFunc_t m_ServiceFunction;
	};
}