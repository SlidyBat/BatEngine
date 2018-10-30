#pragma once

#include <DirectXMath.h>

class Camera
{
public:
	Camera( const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot, float fov = 90.0f, float ar = 1.3333f, float screen_near = 0.1f, float screen_far = 1000.0f );
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

	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetProjectionMatrix() const;
private:
	void UpdateProjectionMatrix();
	void UpdateViewMatrix();
private:
	float m_flFOV;
	float m_flAspectRatio;
	float m_flScreenNear;
	float m_flScreenFar;

	DirectX::XMFLOAT3 m_vecPosition;
	DirectX::XMFLOAT3 m_angRotation;

	DirectX::XMFLOAT3 m_vecForward;
	DirectX::XMFLOAT3 m_vecRight;

	DirectX::XMMATRIX m_matProjMatrix;
	DirectX::XMMATRIX m_matViewMatrix;
};