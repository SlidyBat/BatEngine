#pragma once

#include <functional>

// Helper macro that creates an RAII class to auto-init/shutdown a system
// The pre-requisite is that the system class contains static Initialize/Shutdown functions
#define BAT_INIT_SYSTEM(name) EngineSystem _Bat_##name##_System = EngineSystem( #name, name::Initialize, name::Shutdown );

namespace Bat
{
	using SystemInitFunc_t = std::function<void()>;
	using SystemDestroyFunc_t = std::function<void()>;

	class EngineSystem
	{
	public:
		EngineSystem( const std::string& name, SystemInitFunc_t init, SystemDestroyFunc_t destroy );
		~EngineSystem();
	private:
		std::string m_szName;
		SystemDestroyFunc_t m_DestroyFunction;
	};

	class EngineSystemsInit
	{
	
	};
}