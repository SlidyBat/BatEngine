#include "PCH.h"
#include "AnimationSystem.h"

#include "CoreEntityComponents.h"
#include "PhysicsComponent.h"
#include "AnimationComponent.h"
#include "AnimationSkeleton.h"
#include "AnimationChannel.h"

namespace Bat
{
	void AnimationSystem::Update( EntityManager& world, float dt )
	{
		for( Entity ent : world )
		{
			if( ent.Has<AnimationComponent>() )
			{
				ASSERT( !ent.Has<PhysicsComponent>(), "Physics components interferes with animation component" );

				auto& anim = ent.Get<AnimationComponent>();
				auto& t = ent.Get<TransformComponent>();

				SkeletonPose poses[8];
				float weights[8];

				ASSERT( anim.states.size() < 8, "Can only have 8 active animation states at a time" );

				if( anim.states.empty() )
				{
					anim.current_pose = anim.bind_pose;
				}
				else
				{
					int num_poses = 0;
					for( size_t i = 0; i < anim.states.size(); i++ )
					{
						AnimationState& state = anim.states[i];
						
						state.Update( dt );

						if( Math::CloseEnough( state.GetWeight(), 0.0f ) )
						{
							continue;
						}

						poses[num_poses] = state.GetSample( anim.bind_pose );
						weights[num_poses] = state.GetWeight();
						num_poses++;
					}

					if( num_poses > 0 )
					{
						SkeletonPose blended = SkeletonPose::Blend( poses, weights, num_poses, anim.bind_pose );
						anim.current_pose = std::move( blended );
					}
					else
					{
						anim.current_pose = anim.bind_pose;
					}
				}

				for( BoneNode& node : anim.current_pose.bones )
				{
					auto& transform = node.entity.Get<TransformComponent>();
					transform.SetPosition( node.transform.translation )
						.SetRotation( Math::QuaternionToEulerDeg( node.transform.rotation ) );
				}
			}
		}
	}
}