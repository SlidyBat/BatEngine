#include "PCH.h"
#include "Event.h"

namespace Bat
{
	std::unordered_map<std::type_index, std::vector<EventDispatcher::EventListener>> EventDispatcher::m_GlobalCallbacks;
}