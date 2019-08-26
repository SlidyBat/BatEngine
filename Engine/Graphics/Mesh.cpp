#include "PCH.h"
#include "Mesh.h"

#include "Camera.h"

namespace Bat
{
	Mesh::Mesh( const MeshParameters& params, const std::vector<unsigned int>& indices, const Material& material )
		:
		m_bufIndices( indices ),
		m_Material( material )
	{
		SetData( params );
	}

	void Mesh::Bind( IGPUContext* pContext, IPipeline* pPipeline ) const
	{
		// TODO: support other primitives?
		pContext->SetPrimitiveTopology( PrimitiveTopology::TRIANGLELIST );

		size_t slot = 0;

		if( pPipeline->RequiresVertexAttribute( VertexAttribute::Position ) )
		{
			ASSERT( m_bufPosition->GetVertexCount() > 0, "Shader requires model to have vertex position data" );
			pContext->SetVertexBuffer( m_bufPosition, slot++ );
		}
		if( pPipeline->RequiresVertexAttribute( VertexAttribute::Colour ) )
		{
			ASSERT( m_bufColour->GetVertexCount() > 0, "Shader requires model to have vertex colour data" );
			pContext->SetVertexBuffer( m_bufColour, slot++ );
		}
		if( pPipeline->RequiresVertexAttribute( VertexAttribute::Normal ) )
		{
			ASSERT( m_bufNormal->GetVertexCount() > 0, "Shader requires model to have vertex normal data" );
			pContext->SetVertexBuffer( m_bufNormal, slot++ );
		}
		if( pPipeline->RequiresVertexAttribute( VertexAttribute::UV ) )
		{
			ASSERT( m_bufUV->GetVertexCount() > 0, "Shader requires model to have vertex UV data" );
			pContext->SetVertexBuffer( m_bufUV, slot++ );
		}
		if( pPipeline->RequiresVertexAttribute( VertexAttribute::Tangent ) )
		{
			ASSERT( m_bufTangent->GetVertexCount() > 0, "Shader requires model to have vertex tangent data" );
			pContext->SetVertexBuffer( m_bufTangent, slot++ );
		}
		if( pPipeline->RequiresVertexAttribute( VertexAttribute::Bitangent ) )
		{
			ASSERT( m_bufBitangent->GetVertexCount() > 0, "Shader requires model to have vertex bitangent data" );
			pContext->SetVertexBuffer( m_bufBitangent, slot++ );
		}

		pContext->SetIndexBuffer( m_bufIndices );
	}

	void Mesh::SetData( const MeshParameters& params )
	{
		if( !params.position.empty() )
		{
			m_vecMins = { FLT_MAX, FLT_MAX, FLT_MAX };
			m_vecMaxs = { FLT_MIN, FLT_MIN, FLT_MIN };

			for( const auto& pos : params.position )
			{
				if( pos.x < m_vecMins.x ) m_vecMins.x = pos.x;
				if( pos.y < m_vecMins.y ) m_vecMins.y = pos.y;
				if( pos.z < m_vecMins.z ) m_vecMins.z = pos.z;

				if( pos.x > m_vecMaxs.x ) m_vecMaxs.x = pos.x;
				if( pos.y > m_vecMaxs.y ) m_vecMaxs.y = pos.y;
				if( pos.z > m_vecMaxs.z ) m_vecMaxs.z = pos.z;
			}

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
	}

	void Mesh::SetIndices( const std::vector<unsigned int>& indices )
	{
		m_bufIndices.Reset( indices );
	}

	Material& Mesh::GetMaterial()
	{
		return m_Material;
	}

	size_t Mesh::GetVertexCount() const
	{
		return m_bufPosition->GetVertexCount();
	}

	size_t Mesh::GetIndexCount() const
	{
		return m_bufIndices->GetIndexCount();
	}
}