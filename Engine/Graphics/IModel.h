#pragma once

#include "PCH.h"

#include "Camera.h"
#include "Texture.h"
#include "VertexTypes.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Mesh.h"

namespace Bat
{
	class IPipeline;

	class IModel
	{
	public:
		IModel()
			:
			m_matWorldMatrix( DirectX::XMMatrixIdentity() )
		{}
		virtual ~IModel() = default;

		virtual void Draw( IPipeline* pPipeline ) const = 0;

		DirectX::XMFLOAT3 GetPosition() const
		{
			return m_vecPosition;
		}
		void SetPosition( const float x, const float y, const float z )
		{
			m_vecPosition = { x, y, z };
			UpdateWorldMatrix();
		}
		void SetPosition( const DirectX::XMFLOAT3& pos )
		{
			m_vecPosition = pos;
			UpdateWorldMatrix();
		}
		void MoveBy( const float dx, const float dy, const float dz )
		{
			m_vecPosition.x += dx;
			m_vecPosition.y += dy;
			m_vecPosition.z += dz;
			UpdateWorldMatrix();
		}

		DirectX::XMFLOAT3 GetRotation() const
		{
			return m_angRotation;
		}
		void SetRotation( const float pitch, const float yaw, const float roll )
		{
			m_angRotation = { pitch, yaw, roll };
			UpdateWorldMatrix();
		}
		void SetRotation( const DirectX::XMFLOAT3& ang )
		{
			m_angRotation = ang;
			UpdateWorldMatrix();
		}
		void RotateBy( const float dpitch, const float dyaw, const float droll )
		{
			m_angRotation.x += dpitch;
			m_angRotation.y += dyaw;
			m_angRotation.z += droll;
			UpdateWorldMatrix();
		}
	protected:
		DirectX::XMMATRIX GetWorldMatrix() const
		{
			return m_matWorldMatrix;
		}
		void UpdateWorldMatrix()
		{
			m_matWorldMatrix = DirectX::XMMatrixRotationRollPitchYaw( m_angRotation.x, m_angRotation.y, m_angRotation.z ) *
				DirectX::XMMatrixTranslation( m_vecPosition.x, m_vecPosition.y, m_vecPosition.z );
		}
	protected:
		DirectX::XMFLOAT3 m_vecPosition = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 m_angRotation = { 0.0f, 0.0f, 0.0f };
		DirectX::XMMATRIX m_matWorldMatrix;
	};
}