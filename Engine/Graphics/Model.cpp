#include "PCH.h"
#include "Model.h"

#include "Mesh.h"

namespace Bat
{
	ModelComponent::ModelComponent( std::vector<Resource<Mesh>> pMeshes )
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

	std::vector<Resource<Mesh>>& ModelComponent::GetMeshes()
	{
		return m_pMeshes;
	}

	const std::vector<Resource<Mesh>>& ModelComponent::GetMeshes() const
	{
		return m_pMeshes;
	}

	const Vec3& ModelComponent::GetMins() const
	{
		return m_vecMins;
	}
	const Vec3& ModelComponent::GetMaxs() const
	{
		return m_vecMaxs;
	}
}