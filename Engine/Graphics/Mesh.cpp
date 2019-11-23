#include "PCH.h"
#include "Mesh.h"

#include "Camera.h"
#include "imgui.h"

namespace Bat
{
	Mesh::Mesh( const MeshParameters& params, const std::vector<unsigned int>& indices, const Material& material )
		:
		m_Material( material )
	{
		SetIndices( indices );
		SetData( params );
	}

	void Mesh::Bind( IGPUContext* pContext, IVertexShader* pVertexShader ) const
	{
		pContext->SetPrimitiveTopology( m_Topology );

		size_t slot = 0;

		if( pVertexShader->RequiresVertexAttribute( VertexAttribute::Position ) )
		{
			ASSERT( m_bufPosition->GetVertexCount() > 0, "Shader requires model to have vertex position data" );
			pContext->SetVertexBuffer( m_bufPosition, slot++ );
		}
		if( pVertexShader->RequiresVertexAttribute( VertexAttribute::Colour ) )
		{
			ASSERT( m_bufColour->GetVertexCount() > 0, "Shader requires model to have vertex colour data" );
			pContext->SetVertexBuffer( m_bufColour, slot++ );
		}
		if( pVertexShader->RequiresVertexAttribute( VertexAttribute::Normal ) )
		{
			ASSERT( m_bufNormal->GetVertexCount() > 0, "Shader requires model to have vertex normal data" );
			pContext->SetVertexBuffer( m_bufNormal, slot++ );
		}
		if( pVertexShader->RequiresVertexAttribute( VertexAttribute::UV ) )
		{
			ASSERT( m_bufUV->GetVertexCount() > 0, "Shader requires model to have vertex UV data" );
			pContext->SetVertexBuffer( m_bufUV, slot++ );
		}
		if( pVertexShader->RequiresVertexAttribute( VertexAttribute::Tangent ) )
		{
			ASSERT( m_bufTangent->GetVertexCount() > 0, "Shader requires model to have vertex tangent data" );
			pContext->SetVertexBuffer( m_bufTangent, slot++ );
		}
		if( pVertexShader->RequiresVertexAttribute( VertexAttribute::Bitangent ) )
		{
			ASSERT( m_bufBitangent->GetVertexCount() > 0, "Shader requires model to have vertex bitangent data" );
			pContext->SetVertexBuffer( m_bufBitangent, slot++ );
		}
		if( pVertexShader->RequiresVertexAttribute( VertexAttribute::BoneId ) )
		{
			ASSERT( m_bufBoneIds->GetVertexCount() > 0, "Shader requires model to have bone id data" );
			pContext->SetVertexBuffer( m_bufBoneIds, slot++ );
		}
		if( pVertexShader->RequiresVertexAttribute( VertexAttribute::BoneWeight ) )
		{
			ASSERT( m_bufBoneWeights->GetVertexCount() > 0, "Shader requires model to have bone weight data" );
			pContext->SetVertexBuffer( m_bufBoneWeights, slot++ );
		}

		if( m_bufIndices )
		{
			pContext->SetIndexBuffer( m_bufIndices );
		}
	}

	void Mesh::SetData( const MeshParameters& params )
	{
		m_vecPositions = params.position;

		if( !params.position.empty() )
		{
			m_Aabb = AABB( m_vecPositions.data(), m_vecPositions.size() );

			m_bufPosition.Reset( params.position );
		}
		if( !params.colour.empty() )
		{
			m_bufColour.Reset( params.colour );
		}
		if( !params.normal.empty() )
		{
			m_bufNormal.Reset( params.normal );
		}
		if( !params.uv.empty() )
		{
			m_bufUV.Reset( params.uv );
		}
		if( !params.tangent.empty() )
		{
			m_bufTangent.Reset( params.tangent );
		}
		if( !params.bitangent.empty() )
		{
			m_bufBitangent.Reset( params.bitangent );
		}
		if( !params.bone_ids.empty() )
		{
			m_bufBoneIds.Reset( params.bone_ids );
		}
		if( !params.bone_weights.empty() )
		{
			m_bufBoneWeights.Reset( params.bone_weights );
		}
	}

	void Mesh::SetIndices( const std::vector<unsigned int>& indices )
	{
		m_bufIndices.Reset( indices );
		m_iIndices = indices;
	}

	Material& Mesh::GetMaterial()
	{
		return m_Material;
	}

	const Material& Mesh::GetMaterial() const
	{
		return m_Material;
	}

	const Vec3* Mesh::GetVertexData() const
	{
		return m_vecPositions.data();
	}

	size_t Mesh::GetVertexCount() const
	{
		return m_vecPositions.size();
	}

	const unsigned int* Mesh::GetIndexData() const
	{
		return m_iIndices.data();
	}

	size_t Mesh::GetIndexCount() const
	{
		return m_iIndices.size();
	}
	void Mesh::DoImGuiMenu()
	{
		ImGui::CheckboxFlags( GetName().c_str(), (unsigned int*)&m_RenderFlags, (unsigned int)RenderFlags::DRAW_BBOX );
	}
}