#pragma once

#include "PCH.h"

#include "Camera.h"
#include "Texture.h"
#include "VertexTypes.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ResourceManager.h"

namespace Bat
{
	class IPipeline;

	class Model
	{
	public:
		Model( std::vector<Resource<Mesh>> pMeshes );

		void Bind();

		std::vector<Resource<Mesh>>& GetMeshes();

		DirectX::XMFLOAT3 GetPosition() const;
		void SetPosition( const float x, const float y, const float z );
		void SetPosition( const DirectX::XMFLOAT3& pos );
		void MoveBy( const float dx, const float dy, const float dz );

		DirectX::XMFLOAT3 GetRotation() const;
		void SetRotation( const float pitch, const float yaw, const float roll );
		void SetRotation( const DirectX::XMFLOAT3& ang );
		void RotateBy( const float dpitch, const float dyaw, const float droll );

		float GetScale() const;
		void SetScale( const float scale );

		DirectX::XMMATRIX GetWorldMatrix() const;
	protected:
		void SetDirty( const bool dirty );
		bool IsDirty() const;

		void UpdateWorldMatrix();
	protected:
		DirectX::XMMATRIX m_matWorldMatrix = DirectX::XMMatrixIdentity();
		DirectX::XMFLOAT3 m_vecPosition = { 0.0f, 0.0f, 0.0f };
		float m_flScale = 1.0f;
		DirectX::XMFLOAT3 m_angRotation = { 0.0f, 0.0f, 0.0f };
		bool m_bDirty = false;

		std::vector<Resource<Mesh>> m_pMeshes;
	};
}