#include "PCH.h"
#include "Model.h"

#include "Mesh.h"
#include "imgui.h"

namespace Bat
{
	ModelComponent::ModelComponent( std::vector<Resource<Mesh>> pMeshes )
		:
		m_pMeshes( std::move( pMeshes ) )
	{
		std::vector<Vec3> points;
		points.reserve( m_pMeshes.size() * 2 );
		for( const auto& pMesh : m_pMeshes )
		{
			const AABB& mesh_aabb = pMesh->GetAABB();
			points.push_back( mesh_aabb.mins );
			points.push_back( mesh_aabb.maxs );
		}

		m_Aabb = AABB( points.data(), points.size() );
	}

	std::vector<Resource<Mesh>>& ModelComponent::GetMeshes()
	{
		return m_pMeshes;
	}

	const std::vector<Resource<Mesh>>& ModelComponent::GetMeshes() const
	{
		return m_pMeshes;
	}
	void ModelComponent::DoImGuiMenu()
	{
		if( ImGui::TreeNode( "Model" ) )
		{
			const auto& meshes = GetMeshes();
			for( const auto& mesh : meshes )
			{
				mesh->DoImGuiMenu();
			}

			ImGui::CheckboxFlags( "Draw BBox", (unsigned int*)&m_RenderFlags, (unsigned int)RenderFlags::DRAW_BBOX );

			ImGui::TreePop();
		}
	}
}