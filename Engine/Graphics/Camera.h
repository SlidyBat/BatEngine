#pragma once

#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	Camera( const Camera& src ) = delete;
	Camera& operator=( const Camera& src ) = delete;
	Camera( Camera&& donor ) = delete;
	Camera& operator=( Camera&& donor ) = delete;

	void MoveBy( const float dx, const float dy, const float dz );
	void MoveBy( const DirectX::XMFLOAT3& pos );
	void SetPosition( const DirectX::XMFLOAT3& pos );
	void SetPosition( const float x, const float y, const float z );
	DirectX::XMFLOAT3 GetPosition() const;

	void RotateBy( const float dpitch, const float dyaw, const float droll );
	void RotateBy( const DirectX::XMFLOAT3& rot );
	void SetRotation( const DirectX::XMFLOAT3& rot );
	void SetRotation( const float pitch, const float yaw, const float roll );
	DirectX::XMFLOAT3 GetRotation() const;

	DirectX::XMFLOAT3 GetForwardVector() const;
	DirectX::XMFLOAT3 GetRightVector() const;

	void Render();
	DirectX::XMMATRIX GetViewMatrix() const;
private:
	DirectX::XMFLOAT3 m_vecPosition;
	DirectX::XMFLOAT3 m_angRotation;

	DirectX::XMFLOAT3 m_vecForward;
	DirectX::XMFLOAT3 m_vecRight;

	DirectX::XMMATRIX m_matViewMatrix;
};