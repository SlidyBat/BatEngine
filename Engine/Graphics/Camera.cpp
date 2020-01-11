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
		float gradient_h = tanf( Math::DegToRad( m_flFOV / 2.0f ) );
		float gradient_v = tanf( Math::DegToRad( ( m_flFOV * m_flAspectRatio ) / 2.0f ) );

		float xn = m_flScreenNear * gradient_h;
		float yn = m_flScreenNear * gradient_v;
		float xf = m_flScreenFar * gradient_h;
		float yf = m_flScreenFar * gradient_v;

		corners_out[0] = {  xn,  yn, m_flScreenNear };
		corners_out[1] = { -xn,  yn, m_flScreenNear };
		corners_out[2] = { -xn, -yn, m_flScreenNear };
		corners_out[3] = {  xn, -yn, m_flScreenNear };
		
		corners_out[4] = {  xn,  yn, m_flScreenFar };
		corners_out[5] = { -xn,  yn, m_flScreenFar };
		corners_out[6] = { -xn, -yn, m_flScreenFar };
		corners_out[7] = {  xn, -yn, m_flScreenFar };
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

		lookAtVec = XMVector3TransformNormal( lookAtVec, rotationMatrix );
		upVec = XMVector3TransformNormal( upVec, rotationMatrix );

		m_matViewMatrix = XMMatrixLookToLH( positionVec, lookAtVec, upVec );

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

	void Camera::UpdateFrustum()
	{
		auto transform = m_matViewMatrix * m_matProjMatrix;
		m_Frustum = Frustum( transform );
	}

	void Camera::UpdateProjectionMatrix()
	{
		m_matProjMatrix = DirectX::XMMatrixPerspectiveFovLH( Math::DegToRad( m_flFOV ), m_flAspectRatio, m_flScreenNear, m_flScreenFar );
	}
}