#include "Camera.h"

Camera::Camera()
	:
	m_vecPosition( 0.0f, 0.0f, 0.0f ),
	m_angRotation( 0.0f, 0.0f, 0.0f )
{
}

void Camera::SetPosition( float x, float y, float z )
{
	m_vecPosition = { x, y, z };
}

DirectX::XMFLOAT3 Camera::GetPosition() const
{
	return m_vecPosition;
}

void Camera::SetRotation( float pitch, float yaw, float roll )
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
		m_angRotation.x * 0.0174532925f,
		m_angRotation.y * 0.0174532925f,
		m_angRotation.z * 0.0174532925f );

	lookAtVec = XMVector3TransformCoord( lookAtVec, rotationMatrix );
	upVec = XMVector3TransformCoord( upVec, rotationMatrix );

	lookAtVec += positionVec;

	m_matViewMatrix = XMMatrixLookAtLH( positionVec, lookAtVec, upVec );
}

DirectX::XMMATRIX Camera::GetViewMatrix() const
{
	return m_matViewMatrix;
}
