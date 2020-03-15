#include "PCH.h"
#include "BehaviourTree.h"

namespace Bat
{
	BAT_COMPONENT_BEGIN( BehaviourTree );
	BAT_COMPONENT_END();

	void BehaviourTreeSystem::Update( EntityManager& world )
	{
		for( Entity e : world )
		{
			if( e.Has<BehaviourTree>() )
			{
				auto& tree = e.Get<BehaviourTree>();
				if( tree.root_node )
				{
					tree.root_node->Go( e );
				}
			}
		}
	}
}
