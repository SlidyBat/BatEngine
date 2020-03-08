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
		TransformComponent( const Mat4& mat )
		{
			auto vtransform = DirectX::XMMatrixTranspose(
				DirectX::XMLoadFloat4x4( reinterpret_cast<const DirectX::XMFLOAT4X4*>( &mat ) )
			);

			DirectX::XMVECTOR vscale, vrot, vpos;
			DirectX::XMMatrixDecompose( &vscale, &vrot, &vpos, vtransform );

			transform = mat;
			position = vpos;
			rotation = Math::QuaternionToEuler( vrot );
			DirectX::XMStoreFloat( &scale, vscale );
			dirty = false;
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

		const Mat4& GetTransform() const
		{
			if( dirty )
			{
				transform = Mat4::Scale( scale ) *
					Mat4::RotateDeg( rotation.x, rotation.y, rotation.z ) *
					Mat4::Translate( position.x, position.y, position.z );
				dirty = false;
			}

			return transform;
		}
	private:
		mutable Mat4 transform = Mat4::Identity();
		Vec3 position = { 0.0f, 0.0f, 0.0f };
		float scale = 1.0f;
		Vec3 rotation = { 0.0f, 0.0f, 0.0f };
		mutable bool dirty = true;
	};

	struct HierarchyComponent : Component<HierarchyComponent>
	{
		Mat4 abs_transform;
	};

	class HierarchySystem
	{
	public:
		void Update( SceneNode& root_node );
	private:
		std::vector<SceneNode*> m_NodeStack;
		std::vector<Mat4> m_Transforms;
	};
}