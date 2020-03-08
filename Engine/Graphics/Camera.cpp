#include "PCH.h"
#include "Camera.h"

namespace Bat
{
	Camera::Camera( const Vec3& pos, const Vec3& rot, float fov, float ar, float screen_near, float screen_far )
		:
		m_vecPosition( pos ),
		m_angRotation( rot ),
		m_flFOV( fov ),
		m_flAspectRatio( ar ),
		m_flScreenNear( screen_near ),
		m_flScreenFar( screen_far )
	{
		UpdateViewMatrix();
		UpdateProjectionMatrix();
		UpdateFrustum();
	}

	Camera::Camera( float fov, float ar, float screen_near, float screen_far )
		:
		Camera( { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, fov, ar, screen_near, screen_far )
	{}

	Camera Camera::ScreenOrtho()
	{
		Camera cam;
		cam.m_flScreenNear = Graphics::ScreenNear;
		cam.m_flScreenFar = Graphics::ScreenFar;

		const Viewport& vp = gpu->GetContext()->GetViewport();
		cam.m_matViewMatrix = Mat4::OrthoOffCentre( 0.0f, vp.width, vp.height, 0.0f, Graphics::ScreenNear, Graphics::ScreenFar );
		
		cam.UpdateFrustum();

		return cam;
	}

	float Camera::GetFOV() const
	{
		return m_flFOV;
	}

	void Camera::SetFOV( float fov )
	{
		m_flFOV = fov;
	}

	float Camera::GetAspectRatio() const
	{
		return m_flAspectRatio;
	}

	void Camera::SetAspectRatio( float ar )
	{
		m_flAspectRatio = ar;
	}

	float Camera::GetNear() const
	{
		return m_flScreenNear;
	}

	void Camera::SetNear( const float screen_near )
	{
		m_flScreenNear = screen_near;
	}

	float Camera::GetFar() const
	{
		return m_flScreenFar;
	}

	void Camera::SetFar( const float screen_far )
	{
		m_flScreenFar = screen_far;
	}

	void Camera::MoveBy( const float dx, const float dy, const float dz )
	{
		m_vecPosition.x += dx;
		m_vecPosition.y += dy;
		m_vecPosition.z += dz;
	}

	void Camera::MoveBy( const Vec3& pos )
	{
		m_vecPosition.x += pos.x;
		m_vecPosition.y += pos.y;
		m_vecPosition.z += pos.z;
	}

	void Camera::SetPosition( const Vec3& pos )
	{
		m_vecPosition = pos;
	}

	void Camera::SetPosition( const float x, const float y, const float z )
	{
		m_vecPosition = { x, y, z };
	}

	Vec3 Camera::GetPosition() const
	{
		return m_vecPosition;
	}

	void Camera::RotateBy( const float dpitch, const float dyaw, const float droll )
	{
		m_angRotation.x += dpitch;
		m_angRotation.y += dyaw;
		m_angRotation.z += droll;
		WrapAngle();
	}

	void Camera::RotateBy( const Vec3& rot )
	{
		m_angRotation.x += rot.x;
		m_angRotation.y += rot.y;
		m_angRotation.z += rot.z;
		WrapAngle();
	}

	void Camera::SetRotation( const Vec3& rot )
	{
		m_angRotation = rot;
		WrapAngle();
	}

	void Camera::SetRotation( const float pitch, const float yaw, const float roll )
	{
		m_angRotation = { pitch, yaw, roll };
		WrapAngle();
	}

	Vec3 Camera::GetRotation() const
	{
		return m_angRotation;
	}

	Vec3 Camera::GetForwardVector() const
	{
		return m_vecForward;
	}

	Vec3 Camera::GetRightVector() const
	{
		return m_vecRight;
	}

	Vec3 Camera::GetLookAtVector() const
	{
		static const Vec3 defaultLookAt = { 0.0f, 0.0f, 1.0f };

		Mat4 rotationMatrix = Mat4::RotateDeg( m_angRotation );

		return rotationMatrix * defaultLookAt;
	}

	void Camera::CalculateFrustumCorners( Vec3 corners_out[8] )
	{
		Mat4 inv_vp = Mat4::Inverse( m_matViewMatrix * m_matProjMatrix );

		// Corners in homogeneous clip space
		Vec3 corners[8] =
		{                           //                   7--------6
			{  1.0f, -1.0f, 0.0f }, //                  /|       /|
			{ -1.0f, -1.0f, 0.0f }, //   Y ^           / |      / |
			{  1.0f,  1.0f, 0.0f }, //   | _          3--------2  |
			{ -1.0f,  1.0f, 0.0f }, //   | /' Z       |  |     |  |
			{  1.0f, -1.0f, 1.0f }, //   |/           |  5-----|--4
			{ -1.0f, -1.0f, 1.0f }, //   + ---> X     | /      | /
			{  1.0f,  1.0f, 1.0f }, //                |/       |/
			{ -1.0f,  1.0f, 1.0f }  //                1--------0
		};

		// Convert to world space
		for( int i = 0; i < 8; ++i )
		{
			corners_out[i] = inv_vp * corners[i];
		}
	}

	const Mat4& Camera::GetViewMatrix() const
	{
		return m_matViewMatrix;
	}

	const Mat4& Camera::GetProjectionMatrix() const
	{
		return m_matProjMatrix;
	}

	void Camera::Render()
	{
		UpdateViewMatrix();
		UpdateProjectionMatrix();
		UpdateFrustum();
	}

	void Camera::UpdateViewMatrix()
	{
		Mat4 rotationMatrix = Mat4::RotateDeg( m_angRotation );

		Vec3 dir = Mat4::TransformNormal( rotationMatrix, { 0.0f, 0.0f, 1.0f } );
		Vec3 up = Mat4::TransformNormal( rotationMatrix, { 0.0f, 1.0f, 0.0f } );

		m_matViewMatrix = Mat4::LookTo( m_vecPosition, dir, up );

		// direction vector calculations
		Mat4 vecRotationMatrix = Mat4::RotateDeg( 0.0f, m_angRotation.y, 0.0f );
		m_vecForward = vecRotationMatrix * Vec3{ 0.0f, 0.0f, 1.0f };
		m_vecRight = vecRotationMatrix * Vec3{ 1.0f, 0.0f, 0.0f };
	}

	void Camera::UpdateFrustum()
	{
		auto transform = m_matViewMatrix * m_matProjMatrix;
		m_Frustum = Frustum( transform );
	}

	void Camera::WrapAngle()
	{
		m_angRotation.x = Math::NormalizeAngle( m_angRotation.x );
		m_angRotation.y = Math::NormalizeAngle( m_angRotation.y );
		m_angRotation.z = Math::NormalizeAngle( m_angRotation.z );
	}

	void Camera::UpdateProjectionMatrix()
	{
		m_matProjMatrix = Mat4::PerspectiveFov( Math::DegToRad( m_flFOV ), m_flAspectRatio, m_flScreenNear, m_flScreenFar );
	}
}