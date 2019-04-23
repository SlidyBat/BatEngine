#include "PCH.h"
#include "Model.h"

#include "Mesh.h"

namespace Bat
{
	Model::Model( std::vector<Resource<Mesh>> pMeshes )
		:
		m_pMeshes( std::move( pMeshes ) )
	{}

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

	DirectX::XMFLOAT3 Model::GetPosition() const
	{
		return m_vecPosition;
	}

	void Model::SetPosition( const float x, const float y, const float z )
	{
		m_vecPosition = { x, y, z };
		SetDirty( true );
	}

	void Model::SetPosition( const DirectX::XMFLOAT3& pos )
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
	DirectX::XMFLOAT3 Model::GetRotation() const
	{
		return m_angRotation;
	}

	void Model::SetRotation( const float pitch, const float yaw, const float roll )
	{
		m_angRotation = { pitch, yaw, roll };
		SetDirty( true );
	}

	void Model::SetRotation( const DirectX::XMFLOAT3& ang )
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