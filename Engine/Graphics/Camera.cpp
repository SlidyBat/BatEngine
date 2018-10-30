#include "Camera.h"

Camera::Camera()
	:
	m_vecPosition( 0.0f, 0.0f, 0.0f ),
	m_angRotation( 0.0f, 0.0f, 0.0f )
{
}

void Camera::MoveBy( const float dx, const float dy, const float dz )
{
	m_vecPosition.x += dx;
	m_vecPosition.y += dy;
	m_vecPosition.z += dz;
}

void Camera::SetPosition( const DirectX::XMFLOAT3& pos )
{
	m_vecPosition = pos;
}

void Camera::SetPosition( const float x, const float y, const float z )
{
	m_vecPosition = { x, y, z };
}

DirectX::XMFLOAT3 Camera::GetPosition() const
{
	return m_vecPosition;
}

void Camera::RotateBy( const float dpitch, const float dyaw, const float droll )
{
	m_angRotation.x += dpitch;
	m_angRotation.y += dyaw;
	m_angRotation.z += droll;
}

void Camera::SetRotation( const DirectX::XMFLOAT3& rot )
{
	m_angRotation = rot;
}

void Camera::SetRotation( const float pitch, const float yaw, const float roll )
{
	m_angRotation = { pitch, yaw, roll };
}

DirectX::XMFLOAT3 Camera::GetRotation() const
{
	return m_angRotation;
}

void Camera::Render()
{
	using namespace DirectX;

	static const XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
	XMVECTOR upVec = XMLoadFloat3( &up );

	XMVECTOR positionVec = XMLoadFloat3( &m_vecPosition );
	
	static const XMFLOAT3 defaultLookAt = { 0.0f, 0.0f, 1.0f };
	XMVECTOR lookAtVec = XMLoadFloat3( &defaultLookAt );

	// in radians
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(
		m_angRotation.x * (XM_PI / 180.0f),
		m_angRotation.y * (XM_PI / 180.0f),
		m_angRotation.z * (XM_PI / 180.0f) );

	lookAtVec = XMVector3TransformCoord( lookAtVec, rotationMatrix );
	upVec = XMVector3TransformCoord( upVec, rotationMatrix );

	lookAtVec += positionVec;

	m_matViewMatrix = XMMatrixLookAtLH( positionVec, lookAtVec, upVec );
}

DirectX::XMMATRIX Camera::GetViewMatrix() const
{
	return m_matViewMatrix;
}
