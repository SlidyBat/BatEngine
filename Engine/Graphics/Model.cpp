#include "PCH.h"
#include "Model.h"

#include "Mesh.h"

namespace Bat
{
	Model::Model( std::vector<Resource<Mesh>> pMeshes )
		:
		m_pMeshes( std::move( pMeshes ) )
	{
		m_vecMins = { FLT_MAX, FLT_MAX, FLT_MAX };
		m_vecMaxs = { FLT_MIN, FLT_MIN, FLT_MIN };

		for( const auto& pMesh : m_pMeshes )
		{
			const Vec3& mesh_mins = pMesh->GetMins();
			const Vec3& mesh_maxs = pMesh->GetMins();

			if( mesh_mins.x < m_vecMins.x ) m_vecMins.x = mesh_mins.x;
			if( mesh_mins.y < m_vecMins.y ) m_vecMins.y = mesh_mins.y;
			if( mesh_mins.z < m_vecMins.z ) m_vecMins.z = mesh_mins.z;
			if( mesh_maxs.x < m_vecMins.x ) m_vecMins.x = mesh_maxs.x;
			if( mesh_maxs.y < m_vecMins.y ) m_vecMins.y = mesh_maxs.y;
			if( mesh_maxs.z < m_vecMins.z ) m_vecMins.z = mesh_maxs.z;

			if( mesh_mins.x > m_vecMaxs.x ) m_vecMaxs.x = mesh_mins.x;
			if( mesh_mins.y > m_vecMaxs.y ) m_vecMaxs.y = mesh_mins.y;
			if( mesh_mins.z > m_vecMaxs.z ) m_vecMaxs.z = mesh_mins.z;
			if( mesh_maxs.x > m_vecMaxs.x ) m_vecMaxs.x = mesh_maxs.x;
			if( mesh_maxs.y > m_vecMaxs.y ) m_vecMaxs.y = mesh_maxs.y;
			if( mesh_maxs.z > m_vecMaxs.z ) m_vecMaxs.z = mesh_maxs.z;
		}
	}

	void Model::Bind()
	{
		if( IsDirty() )
		{
			UpdateWorldMatrix();
			SetDirty( false );
		}
	}

	std::vector<Resource<Mesh>>& Model::GetMeshes()
	{
		return m_pMeshes;
	}

	const Vec3& Model::GetPosition() const
	{
		return m_vecPosition;
	}

	void Model::SetPosition( const float x, const float y, const float z )
	{
		m_vecPosition = { x, y, z };
		SetDirty( true );
	}

	void Model::SetPosition( const Vec3& pos )
	{
		m_vecPosition = pos;
		SetDirty( true );
	}

	void Model::MoveBy( const float dx, const float dy, const float dz )
	{
		m_vecPosition.x += dx;
		m_vecPosition.y += dy;
		m_vecPosition.z += dz;
		SetDirty( true );
	}

	const Vec3& Model::GetRotation() const
	{
		return m_angRotation;
	}

	void Model::SetRotation( const float pitch, const float yaw, const float roll )
	{
		m_angRotation = { pitch, yaw, roll };
		SetDirty( true );
	}

	void Model::SetRotation( const Vec3& ang )
	{
		m_angRotation = ang;
		SetDirty( true );
	}

	void Model::RotateBy( const float dpitch, const float dyaw, const float droll )
	{
		m_angRotation.x += dpitch;
		m_angRotation.y += dyaw;
		m_angRotation.z += droll;
		SetDirty( true );
	}

	float Model::GetScale() const
	{
		return m_flScale;
	}

	void Model::SetScale( const float scale )
	{
		m_flScale = scale;
		SetDirty( true );
	}

	const Vec3& Model::GetMins() const
	{
		return m_vecMins;
	}
	const Vec3& Model::GetMaxs() const
	{
		return m_vecMaxs;
	}

	void Model::SetDirty( const bool dirty )
	{
		m_bDirty = dirty;
	}

	bool Model::IsDirty() const
	{
		return m_bDirty;
	}

	DirectX::XMMATRIX Model::GetWorldMatrix() const
	{
		return m_matWorldMatrix;
	}

	void Model::UpdateWorldMatrix()
	{
		m_matWorldMatrix = DirectX::XMMatrixScaling( m_flScale, m_flScale, m_flScale ) *
			DirectX::XMMatrixRotationRollPitchYaw( m_angRotation.x, m_angRotation.y, m_angRotation.z ) *
			DirectX::XMMatrixTranslation( m_vecPosition.x, m_vecPosition.y, m_vecPosition.z );
	}
}