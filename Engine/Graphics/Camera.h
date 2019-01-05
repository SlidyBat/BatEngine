#pragma once

#include "PCH.h"

namespace Bat
{
	class Camera
	{
	public:
		Camera( const Vec3& pos, const Vec3& rot, float fov = 90.0f, float ar = 4.0f / 3.0f, float screen_near = 0.01f, float screen_far = 10000.0f );
		Camera( float fov = 90.0f, float ar = 4.0f / 3.0f, float screen_near = 0.01f, float screen_far = 10000.0f );
		Camera( const Camera& src ) = delete;
		Camera& operator=( const Camera& src ) = delete;
		Camera( Camera&& donor ) = delete;
		Camera& operator=( Camera&& donor ) = delete;

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

		DirectX::XMMATRIX GetViewMatrix() const;
		DirectX::XMMATRIX GetProjectionMatrix() const;

		void Render();
	private:
		void UpdateProjectionMatrix();
		void UpdateViewMatrix();
	private:
		float m_flFOV = 90.0f;
		float m_flAspectRatio = 1.3333333f;
		float m_flScreenNear = 0.01f;
		float m_flScreenFar = 10000.0f;

		Vec3 m_vecPosition = { 0.0f, 0.0f, 0.0f };
		Vec3 m_angRotation = { 0.0f, 0.0f, 0.0f };

		Vec3 m_vecForward;
		Vec3 m_vecRight;

		DirectX::XMMATRIX m_matProjMatrix;
		DirectX::XMMATRIX m_matViewMatrix;
	};
}