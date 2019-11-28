#include "PCH.h"
#include "AnimationSystem.h"

#include "CoreEntityComponents.h"
#include "PhysicsComponent.h"
#include "AnimationComponent.h"
#include "AnimationSkeleton.h"
#include "AnimationChannel.h"

namespace Bat
{
	void AnimationSystem::Update( EntityManager& world )
	{
		for( Entity ent : world )
		{
			if( ent.Has<AnimationComponent>() )
			{
				ASSERT( !ent.Has<PhysicsComponent>(), "Physics components interferes with animation component" );

				auto& anim = ent.Get<AnimationComponent>();
				auto& t = ent.Get<TransformComponent>();

				const AnimationChannel* channel = anim.GetCurrentChannel();
				if( !channel )
				{
					continue;
				}

				BoneTransform sample = anim.GetSample();
				t.SetTransform( BoneTransform::ToMatrix( sample ) );
			}
		}
	}
}