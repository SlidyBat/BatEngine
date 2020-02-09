#pragma once

#include "MathLib.h"
#include "Entity.h"

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
		{
			SetTransform( transform );
		}

		TransformComponent& SetPosition( float x, float y, float z ) { position = { x, y, z }; dirty = true; return *this; }
		TransformComponent& SetPosition( const Vec3& pos ) { return SetPosition( pos.x, pos.y, pos.z ); }
		const Vec3& GetPosition() const { return position; }
		void MoveBy( const float dx, const float dy, const float dz ) { SetPosition( position + Vec3{ dx, dy, dz } ); }
		void MoveBy( const Vec3& delta ) { MoveBy( delta.x, delta.y, delta.z ); }

		TransformComponent& SetRotation( float pitch, float yaw, float roll ) { rotation = { pitch, yaw, roll }; dirty = true; return *this; }
		TransformComponent& SetRotation( const Vec3& rot ) { return SetRotation( rot.x, rot.y, rot.z ); }
		const Vec3& GetRotation() const { return rotation; }
		void RotateBy( const float dpitch, const float dyaw, const float droll ) { SetRotation( rotation + Vec3{ dpitch, dyaw, droll } ); }
		void RotateBy( const Vec3& delta ) { MoveBy( delta.x, delta.y, delta.z ); }

		TransformComponent& SetScale( float uniform_scale ) { scale = uniform_scale; dirty = true; return *this; }
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

		void SetTransform( DirectX::XMMATRIX new_transform )
		{
			transform = new_transform;

			DirectX::XMVECTOR tran, qrot, vscale;
			DirectX::XMMatrixDecompose( &vscale, &qrot, &tran, transform );

			position = tran;
			rotation = Math::QuaternionToEuler( qrot );
			DirectX::XMStoreFloat( &scale, vscale );

			dirty = false;
		}
	private:
		mutable DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();
		Vec3 position = { 0.0f, 0.0f, 0.0f };
		float scale = 1.0f;
		Vec3 rotation = { 0.0f, 0.0f, 0.0f };
		mutable bool dirty = true;
	};

	struct HierarchyComponent : Component<HierarchyComponent>
	{
		DirectX::XMMATRIX abs_transform;
	};

	class HierarchySystem
	{
	public:
		void Update( SceneNode& root_node );
	private:
		std::vector<SceneNode*> m_NodeStack;
		std::vector<DirectX::XMMATRIX> m_Transforms;
	};
}