#pragma once

#include "MathLib.h"
#include "Scene.h"
#include "BatWinAPI.h"
#include "NavMesh.h"
#include "BehaviourTree.h"
#include "CharacterControllerComponent.h"
#include "CoreEntityComponents.h"
#include "EntityTrace.h"
#include "Globals.h"
#include "DebugDraw.h"

#include <algorithm>

class AiCharacter
{
public:
	void Initialize( Bat::SceneNode& scene, const Bat::Vec3& pos, const Bat::NavMeshSystem& navmesh_system, Bat::Entity target_ent )
	{
		navmesh = &navmesh_system;

		ent = Bat::world.CreateEntity();
		scene.AddChild( ent );

		Bat::CharacterControllerBoxDesc box;
		ent.Get<Bat::TransformComponent>()
			.SetPosition( pos );
		ent.Add<Bat::CharacterControllerComponent>( box );

		auto& behaviour = ent.Add<Bat::BehaviourTree>();
		behaviour.root_node = MakeBehaviour( target_ent );
	}
	Bat::Vec3 GetPosition() const { return ent.Get<Bat::TransformComponent>().GetPosition(); }
	Bat::Entity GetEntity() const { return ent; }
private:
	std::unique_ptr<Bat::BehaviourNode> MakeBehaviour( Bat::Entity target_ent )
	{
		auto sequence = std::make_unique<Bat::SequenceNode>();
		sequence->Add( CheckIfVisible( target_ent ) );
		sequence->Add( Chase( target_ent ) );
		sequence->Add( Gotcha() );
		sequence->Add( WaitUntilGone( target_ent ) );
		return sequence;
	}
	std::unique_ptr<Bat::BehaviourNode> CheckIfVisible( Bat::Entity target_ent )
	{
		return std::make_unique<Bat::ActionNode>( [=]( Bat::Entity e ) {
			const auto& target_transform = target_ent.Get<Bat::TransformComponent>();
			Bat::Vec3 target_pos = target_transform.GetPosition();
			const auto& my_transform = e.Get<Bat::TransformComponent>();
			Bat::Vec3 pos = my_transform.GetPosition();

			Bat::Vec3 dir = ( target_pos - pos ).Normalize();
			Bat::RayCastResult trace = Bat::EntityTrace::RayCast( pos + dir * 0.05f, dir, 100.0f );
			if( trace.hit && trace.entity == target_ent )
			{
				return Bat::BehaviourResult::SUCCEEDED;
			}
			return Bat::BehaviourResult::FAILED;
		} );
	}
	std::unique_ptr<Bat::BehaviourNode> Chase( Bat::Entity target_ent )
	{
		return std::make_unique<Bat::ActionNode>( [=]( Bat::Entity e ) {
			auto& controller = e.Get<Bat::CharacterControllerComponent>();
			const auto& target_transform = target_ent.Get<Bat::TransformComponent>();
			Bat::Vec3 target_pos = target_transform.GetPosition();
			const auto& my_transform = e.Get<Bat::TransformComponent>();
			Bat::Vec3 pos = my_transform.GetPosition();

			if( ( target_pos - pos ).LengthSq() < 1.0f )
			{
				return Bat::BehaviourResult::SUCCEEDED;
			}

			Bat::Vec3 floor_pos = { pos.x, pos.y - 0.35f, pos.z };
			std::vector<Bat::Vec3> path = navmesh->GetPath( 0, floor_pos, target_pos );
			Bat::Vec3 delta = path[1] - path[0];
			float len = delta.Length();

			delta /= len;

			Bat::DebugDraw::Line( pos, pos + delta, Bat::Colours::Green );
			for( size_t i = 0; i < path.size() - 1; i++ )
			{
				Bat::DebugDraw::Line( path[i], path[i + 1], Bat::Colours::White );
			}

			float dist = std::min( len, speed * Bat::g_pGlobals->deltatime );
			controller.Move( delta * dist, Bat::g_pGlobals->deltatime );

			return Bat::BehaviourResult::RUNNING;
		} );
	}
	std::unique_ptr<Bat::BehaviourNode> Gotcha()
	{
		return std::make_unique<Bat::ActionNode>( []( Bat::Entity e ) {
			const auto& my_transform = e.Get<Bat::TransformComponent>();
			Bat::Vec3 pos = my_transform.GetPosition();
			snd->Play( "Assets/Ignore/gotcha.wav" );
			return Bat::BehaviourResult::SUCCEEDED;
		} );
	}
	std::unique_ptr<Bat::BehaviourNode> WaitUntilGone( Bat::Entity target_ent )
	{
		auto invert = std::make_unique<Bat::InverseNode>();
		invert->SetChild( CheckIfVisible( target_ent ) );

		auto loop = std::make_unique<Bat::LoopUntilSuccessNode>();
		loop->SetChild( std::move( invert ) );
		return loop;
	}
private:
	float speed = 5.0f;
	Bat::Entity ent;
	Bat::Vec3 target_ent;
	bool going = false;
	const Bat::NavMeshSystem* navmesh = nullptr;
};