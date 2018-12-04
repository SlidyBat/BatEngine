#include "Camera.h"
#include "Event.h"
#include <functional>

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
	}

	Camera::Camera( float fov, float ar, float screen_near, float screen_far )
		:
		Camera( { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, fov, ar, screen_near, screen_far )
	{}

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
	}

	void Camera::RotateBy( const Vec3& rot )
	{
		m_angRotation.x += rot.x;
		m_angRotation.y += rot.y;
		m_angRotation.z += rot.z;
	}

	void Camera::SetRotation( const Vec3& rot )
	{
		m_angRotation = rot;
	}

	void Camera::SetRotation( const float pitch, const float yaw, const float roll )
	{
		m_angRotation = { pitch, yaw, roll };
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
	}

	void Camera::UpdateViewMatrix()
	{
		using namespace DirectX;

		// view matrix calculations
		static const XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
		XMVECTOR upVec = XMLoadFloat3( &up );

		XMVECTOR positionVec = XMLoadFloat3( &m_vecPosition );

		static const XMFLOAT3 defaultLookAt = { 0.0f, 0.0f, 1.0f };
		XMVECTOR lookAtVec = XMLoadFloat3( &defaultLookAt );

		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(
			Math::DegToRad( m_angRotation.x ),
			Math::DegToRad( m_angRotation.y ),
			Math::DegToRad( m_angRotation.z ) );

		lookAtVec = XMVector3TransformCoord( lookAtVec, rotationMatrix );
		upVec = XMVector3TransformCoord( upVec, rotationMatrix );

		lookAtVec += positionVec;

		m_matViewMatrix = XMMatrixLookAtLH( positionVec, lookAtVec, upVec );

		static const XMFLOAT4 forward_vector = { 0.0f, 0.0f, 1.0f, 0.0f };
		static const XMFLOAT4 right_vector = { 1.0f, 0.0f, 0.0f, 0.0f };
		static const XMFLOAT4 up_vector = { 0.0f, 1.0f, 0.0f, 0.0f };

		// direction vector calculations
		XMMATRIX vecRotationMatrix = XMMatrixRotationRollPitchYaw( 0.0f, Math::DegToRad( m_angRotation.y ), 0.0f );
		auto forward = XMVector3TransformCoord( XMLoadFloat4( &forward_vector ), vecRotationMatrix );
		auto right = XMVector3TransformCoord( XMLoadFloat4( &right_vector ), vecRotationMatrix );

		XMStoreFloat3( &m_vecForward, forward );
		XMStoreFloat3( &m_vecRight, right );
	}

	void Camera::UpdateProjectionMatrix()
	{
		m_matProjMatrix = DirectX::XMMatrixPerspectiveFovLH( Math::DegToRad( m_flFOV ), m_flAspectRatio, m_flScreenNear, m_flScreenFar );
	}
}