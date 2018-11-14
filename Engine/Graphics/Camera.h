#pragma once

#include <DirectXMath.h>
#include "MathLib.h"

namespace Bat
{
	class Camera
	{
	public:
		Camera( const Vec3& pos, const Vec3& rot, float fov = 90.0f, float ar = 1.3333f, float screen_near = 0.1f, float screen_far = 1000.0f );
		Camera( float fov = 90.0f, float ar = 1.33333f, float screen_near = 0.1f, float screen_far = 1000.0f );
		Camera( const Camera& src ) = delete;
		Camera& operator=( const Camera& src ) = delete;
		Camera( Camera&& donor ) = delete;
		Camera& operator=( Camera&& donor ) = delete;

		float GetFOV() const;
		void SetFOV( float fov );
		float GetAspectRatio() const;
		void SetAspectRatio( float ar );

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
		float m_flFOV;
		float m_flAspectRatio;
		float m_flScreenNear;
		float m_flScreenFar;

		Vec3 m_vecPosition;
		Vec3 m_angRotation;

		Vec3 m_vecForward;
		Vec3 m_vecRight;

		DirectX::XMMATRIX m_matProjMatrix;
		DirectX::XMMATRIX m_matViewMatrix;
	};
}