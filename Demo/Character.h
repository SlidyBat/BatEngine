#pragma once

#include "MathLib.h"
#include "Scene.h"
#include "CoreEntityComponents.h"
#include "CharacterControllerComponent.h"

class Character
{
public:
	void Initialize( Bat::SceneNode& scene, const Bat::Vec3& pos )
	{
		character = Bat::world.CreateEntity();
		scene.AddChild( character );

		Bat::CharacterControllerBoxDesc box;
		character.Get<Bat::TransformComponent>()
			.SetPosition( pos );
		character.Add<Bat::CharacterControllerComponent>( box );
	}

	void Update( float dt )
	{
		auto& controller = character.Get<Bat::CharacterControllerComponent>();

		velocity += Bat::Vec3{ 0.0f, -9.8f, 0.0f } *dt;

		Bat::PhysicsControllerCollisionFlags flags = controller.Move( velocity * dt + disp, dt );
		if( ( flags & Bat::CONTROLLER_COLLISION_DOWN ) != Bat::CONTROLLER_COLLISION_NONE )
		{
			on_ground = true;
			velocity.y = 0.0f;
		}
		else
		{
			on_ground = false;
		}

		if( ( flags & Bat::CONTROLLER_COLLISION_UP ) != Bat::CONTROLLER_COLLISION_NONE && velocity.y > 0.0f )
		{
			velocity.y = 0.0f;
		}
		if( ( flags & Bat::CONTROLLER_COLLISION_SIDES ) != Bat::CONTROLLER_COLLISION_NONE )
		{
			velocity.x = 0.0f;
			velocity.z = 0.0f;
		}

		disp = { 0.0f, 0.0f, 0.0f };
	}
	void MoveBy( const Bat::Vec3& dpos )
	{
		disp += dpos;
	}
	void RotateBy( const Bat::Vec3& drot )
	{
		auto& t = character.Get<Bat::TransformComponent>();
		t.SetRotation( Bat::Math::NormalizeAngleDeg( t.GetRotation() + drot ) );
	}
	Bat::Vec3 GetPosition()
	{
		auto& t = character.Get<Bat::TransformComponent>();
		return t.GetPosition();
	}
	Bat::Vec3 GetRotation()
	{
		auto& t = character.Get<Bat::TransformComponent>();
		return t.GetRotation();
	}
	Bat::Entity GetEntity() const { return character; }
	void Jump()
	{
		if( on_ground )
		{
			velocity.y += 5.0f;
			on_ground = false;
		}
	}
private:
	Bat::Entity character;
	Bat::Vec3 disp = { 0.0f, 0.0f, 0.0f };
	Bat::Vec3 velocity = { 0.0f, 0.0f, 0.0f };
	bool on_ground = true;
};