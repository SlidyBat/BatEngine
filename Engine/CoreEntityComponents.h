#pragma once

#include "MathLib.h"
#include "Entity.h"
#include "Model.h"

namespace Bat
{
	struct NameComponent : public Component<NameComponent>
	{
		NameComponent( std::string name )
			:
			name( std::move( name ) )
		{}

		std::string name;
	};

	class TransformComponent : public Component<TransformComponent>
	{
	public:
		TransformComponent() = default;
		TransformComponent( DirectX::XMMATRIX transform )
			:
			transform( transform )
		{}

		TransformComponent& SetPosition( float x, float y, float z ) { position = { x, y, z }; dirty = true; return *this; }
		TransformComponent& SetPosition( const Vec3& pos ) { return SetPosition( pos.x, pos.y, pos.z ); }
		const Vec3& GetPosition() const { return position; }

		TransformComponent& SetRotation( float pitch, float yaw, float roll ) { position = { pitch, yaw, roll }; dirty = true; return *this; }
		TransformComponent& SetRotation( const Vec3& rot ) { return SetPosition( rot.x, rot.y, rot.z ); }
		const Vec3& GetRotation() const { return position; }

		TransformComponent& SetScale( float uniform_scale ) { scale = uniform_scale; return *this; }
		float GetScale() const { return scale; }

		const DirectX::XMMATRIX& GetTransform() const
		{
			if( dirty )
			{
				transform = DirectX::XMMatrixScaling( scale, scale, scale ) *
					DirectX::XMMatrixRotationRollPitchYaw( rotation.x, rotation.y, rotation.z ) *
					DirectX::XMMatrixTranslation( position.x, position.y, position.z );
				dirty = false;
			}

			return transform;
		}
	private:
		mutable DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();
		Vec3 position = { 0.0f, 0.0f, 0.0f };
		float scale = 1.0f;
		Vec3 rotation = { 0.0f, 0.0f, 0.0f };
		mutable bool dirty = true;
	};

	struct ModelComponent : public Component<ModelComponent>
	{
		ModelComponent( const std::vector<Resource<Mesh>>& meshes )
			:
			model( meshes )
		{}

		Model model;
	};
}