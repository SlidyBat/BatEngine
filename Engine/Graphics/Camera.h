#pragma once

#include "Core/Entity.h"
#include "Util/Frustum.h"
#include "Renderer.h"

namespace Bat
{
	class Camera
	{
	public:
		Camera( const Vec3& pos, const Vec3& rot, float fov = 90.0f, float ar = 4.0f / 3.0f, float screen_near = Renderer::ScreenNear, float screen_far = Renderer::ScreenFar );
		Camera( float fov = 90.0f, float ar = 4.0f / 3.0f, float screen_near = Renderer::ScreenNear, float screen_far = Renderer::ScreenFar );
		
		static Camera ScreenOrtho();

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
		// Gets frustum corners in world space
		void CalculateFrustumCorners( Vec3 corners_out[8] );

		const Mat4& GetViewMatrix() const;
		const Mat4& GetProjectionMatrix() const;

		void Render();
	private:
		void UpdateProjectionMatrix();
		void UpdateViewMatrix();
		void UpdateFrustum();
		void WrapAngle();
	private:
		Frustum m_Frustum;

		float m_flFOV = 90.0f;
		float m_flAspectRatio = 1.3333333f;
		float m_flScreenNear = Renderer::ScreenNear;
		float m_flScreenFar = Renderer::ScreenFar;

		Vec3 m_vecPosition = { 0.0f, 0.0f, 0.0f };
		Vec3 m_angRotation = { 0.0f, 0.0f, 0.0f };

		Vec3 m_vecForward;
		Vec3 m_vecRight;

		Mat4 m_matProjMatrix = Mat4::Identity();
		Mat4 m_matViewMatrix = Mat4::Identity();
	};
}