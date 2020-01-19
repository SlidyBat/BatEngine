#include "PCH.h"
#include "Camera.h"

namespace Bat
{
	using namespace DirectX;

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
		cam.m_matProjMatrix = DirectX::XMMatrixOrthographicOffCenterLH( 0.0f, vp.width, vp.height, 0.0f, Graphics::ScreenNear, Graphics::ScreenFar );
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
		static const XMFLOAT3 defaultLookAt = { 0.0f, 0.0f, 1.0f };
		XMVECTOR lookAtVec = XMLoadFloat3( &defaultLookAt );

		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(
			Math::DegToRad( m_angRotation.x ),
			Math::DegToRad( m_angRotation.y ),
			Math::DegToRad( m_angRotation.z ) );

		lookAtVec = XMVector3TransformCoord( lookAtVec, rotationMatrix );

		Vec3 res;
		XMStoreFloat3( &res, lookAtVec );
		return res;
	}

	void Camera::CalculateFrustumCorners( Vec3 corners_out[8] )
	{
		XMMATRIX inv_vp = DirectX::XMMatrixInverse( nullptr, m_matViewMatrix * m_matProjMatrix );

		// Corners in homogeneous clip space
		XMVECTOR corners[8] =
		{                                            //                   7--------6
			XMVectorSet(  1.0f, -1.0f, 0.0f, 1.0f ), //                  /|       /|
			XMVectorSet( -1.0f, -1.0f, 0.0f, 1.0f ), //   Y ^           / |      / |
			XMVectorSet(  1.0f,  1.0f, 0.0f, 1.0f ), //   | _          3--------2  |
			XMVectorSet( -1.0f,  1.0f, 0.0f, 1.0f ), //   | /' Z       |  |     |  |
			XMVectorSet(  1.0f, -1.0f, 1.0f, 1.0f ), //   |/           |  5-----|--4
			XMVectorSet( -1.0f, -1.0f, 1.0f, 1.0f ), //   + ---> X     | /      | /
			XMVectorSet(  1.0f,  1.0f, 1.0f, 1.0f ), //                |/       |/
			XMVectorSet( -1.0f,  1.0f, 1.0f, 1.0f ), //                1--------0
		};

		// Convert to world space
		for( int i = 0; i < 8; ++i )
		{
			corners_out[i] = XMVector3TransformCoord( corners[i], inv_vp );
		}
	}

	DirectX::XMMATRIX Camera::GetViewMatrix() const
	{
		return m_matViewMatrix;
	}

	DirectX::XMMATRIX Camera::GetProjectionMatrix() const
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
		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(
			Math::DegToRad( m_angRotation.x ),
			Math::DegToRad( m_angRotation.y ),
			Math::DegToRad( m_angRotation.z ) );

		XMVECTOR lookat = XMVector3TransformNormal( XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ), rotationMatrix );
		XMVECTOR up = XMVector3TransformNormal( XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ), rotationMatrix );

		m_matViewMatrix = XMMatrixLookToLH( m_vecPosition, lookat, up );

		// direction vector calculations
		XMMATRIX vecRotationMatrix = XMMatrixRotationRollPitchYaw( 0.0f, Math::DegToRad( m_angRotation.y ), 0.0f );
		m_vecForward = XMVector3TransformCoord( XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ), vecRotationMatrix );
		m_vecRight = XMVector3TransformCoord( XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f ), vecRotationMatrix );
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
		m_matProjMatrix = DirectX::XMMatrixPerspectiveFovLH( Math::DegToRad( m_flFOV ), m_flAspectRatio, m_flScreenNear, m_flScreenFar );
	}
}