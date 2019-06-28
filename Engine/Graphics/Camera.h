#pragma once

#include "Frustum.h"
#include "Graphics.h"
#include "Entity.h"

namespace Bat
{
	class Camera
	{
	public:
		Camera( const Vec3& pos, const Vec3& rot, float fov = 90.0f, float ar = 4.0f / 3.0f, float screen_near = Graphics::ScreenNear, float screen_far = Graphics::ScreenFar );
		Camera( float fov = 90.0f, float ar = 4.0f / 3.0f, float screen_near = Graphics::ScreenNear, float screen_far = Graphics::ScreenFar );

		float GetFOV() const;
		void SetFOV( float fov );
		float GetAspectRatio() const;
		void SetAspectRatio( float ar );
		float GetNear() const;
		void SetNear( const float screen_near );
		float GetFar() const;
		void SetFar( const float screen_far );

		void MoveBy( const float dx, const float dy, const float dz );
		void MoveBy( const Vec3& pos );
		void SetPosition( const Vec3& pos );
		void SetPosition( const float x, const float y, const float z );
		Vec3 GetPosition() const;

		void RotateBy( const float dpitch, const float dyaw, const float droll );
		void RotateBy( const Vec3& rot );
		void SetRotation( const Vec3& rot );
		void SetRotation( const float pitch, const float yaw, const float roll );
		Vec3 GetRotation() const;

		Vec3 GetForwardVector() const;
		Vec3 GetRightVector() const;
		Vec3 GetLookAtVector() const;

		const Frustum& GetFrustum() const { return m_Frustum; }

		DirectX::XMMATRIX GetViewMatrix() const;
		DirectX::XMMATRIX GetProjectionMatrix() const;

		void Render();
	private:
		void UpdateProjectionMatrix();
		void UpdateViewMatrix();
		void UpdateFrustum();
	private:
		Frustum m_Frustum;

		float m_flFOV = 90.0f;
		float m_flAspectRatio = 1.3333333f;
		float m_flScreenNear = Graphics::ScreenNear;
		float m_flScreenFar = Graphics::ScreenFar;

		Vec3 m_vecPosition = { 0.0f, 0.0f, 0.0f };
		Vec3 m_angRotation = { 0.0f, 0.0f, 0.0f };

		Vec3 m_vecForward;
		Vec3 m_vecRight;

		DirectX::XMMATRIX m_matProjMatrix;
		DirectX::XMMATRIX m_matViewMatrix;
	};

	struct CameraComponent : public Component<CameraComponent>
	{
		CameraComponent( Camera* camera )
			:
			camera( camera )
		{}

		Camera* camera;
	};
}