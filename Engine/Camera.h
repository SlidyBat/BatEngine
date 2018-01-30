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

	void SetPosition( float x, float y, float z );
	DirectX::XMFLOAT3 GetPosition() const;

	void SetRotation( float pitch, float yaw, float roll );
	DirectX::XMFLOAT3 GetRotation() const;

	void Render();
	DirectX::XMMATRIX GetViewMatrix() const;
private:
	DirectX::XMFLOAT3 m_vecPosition;
	DirectX::XMFLOAT3 m_angRotation;

	DirectX::XMMATRIX m_matViewMatrix;
};