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

		void SetName( const std::string& name ) { m_szName = name; }
		std::string GetName() const { return m_szName; }

		void Bind();

		std::vector<Resource<Mesh>>& GetMeshes();

		const Vec3& GetPosition() const;
		void SetPosition( const float x, const float y, const float z );
		void SetPosition( const Vec3& pos );
		void MoveBy( const float dx, const float dy, const float dz );

		const Vec3& GetRotation() const;
		void SetRotation( const float pitch, const float yaw, const float roll );
		void SetRotation( const Vec3& ang );
		void RotateBy( const float dpitch, const float dyaw, const float droll );

		float GetScale() const;
		void SetScale( const float scale );

		// Returns mins of model in object space
		const Vec3& GetMins() const;
		// Returns maxs of model in object space
		const Vec3& GetMaxs() const;

		DirectX::XMMATRIX GetWorldMatrix() const;
	protected:
		void SetDirty( const bool dirty );
		bool IsDirty() const;

		void UpdateWorldMatrix();
	protected:
		DirectX::XMMATRIX m_matWorldMatrix = DirectX::XMMatrixIdentity();
		Vec3 m_vecPosition = { 0.0f, 0.0f, 0.0f };
		float m_flScale = 1.0f;
		Vec3 m_angRotation = { 0.0f, 0.0f, 0.0f };
		bool m_bDirty = false;
		Vec3 m_vecMins;
		Vec3 m_vecMaxs;

		std::vector<Resource<Mesh>> m_pMeshes;

		std::string m_szName;
	};
}