#pragma once

#include <BatEngine.h>
#include "Character.h"

class MoveableCharacter
{
public:
	void Initialize( Bat::SceneNode& scene, const Bat::Vec3& pos )
	{
		character.Initialize( scene, pos );
	}
	void Update( const Bat::Input& input, float dt )
	{
		Bat::Vec3 rotation = character.GetRotation();

		Bat::Vec3 forward, right;
		Bat::Math::AngleVectors( rotation, &forward, &right, nullptr );

		Bat::Vec3 disp = { 0.0f, 0.0f, 0.0f };
		if( input.IsKeyDown( 'A' ) )
		{
			disp += -right;
		}
		if( input.IsKeyDown( 'D' ) )
		{
			disp += right;
		}
		if( input.IsKeyDown( 'W' ) )
		{
			disp += forward;
		}
		if( input.IsKeyDown( 'S' ) )
		{
			disp += -forward;
		}
		if( disp.LengthSq() > 0.01f )
		{
			disp = disp.Normalized() * speed;
		}
		character.MoveBy( disp * dt );

		if( input.IsKeyDown( VK_SPACE ) )
		{
			character.Jump();
		}

		if( input.IsMouseButtonDown( Bat::Input::MouseButton::Left ) )
		{
			const Bat::Vei2& delta = input.GetMouseDelta();
			const float deltayaw = (float)delta.x;
			const float deltapitch = (float)delta.y;

			character.RotateBy( Bat::Vec3{ deltapitch, deltayaw, 0.0f } *0.5f );
		}

		character.Update( dt );
	}
	Bat::Vec3 GetPosition() { return character.GetPosition(); }
	Bat::Vec3 GetRotation() { return character.GetRotation(); }
	Bat::Entity GetEntity() const { return character.GetEntity(); }
private:
	float speed = 5.0f;
	Character character;
};