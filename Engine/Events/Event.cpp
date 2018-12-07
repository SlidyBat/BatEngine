#include "PCH.h"
#include "Event.h"

namespace Bat
{
	std::unordered_map<std::type_index, std::vector<std::function<void( const IEvent* )>>> EventManager::s_mapCallbacks;
}
