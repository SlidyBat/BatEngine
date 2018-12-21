#include "PCH.h"
#include "Mesh.h"

#include "RenderContext.h"
#include "Camera.h"

namespace Bat
{
	Mesh::Mesh( const MeshParameters& params, const std::vector<int>& indices, const Material& material )
		:
		m_bufIndices( indices ),
		m_Material( material )
	{
		SetData( params );
	}

	void Mesh::Bind( IPipeline* pPipeline ) const
	{
		// TODO: support other primitives?
		RenderContext::GetDeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		// bind buffers
		std::vector<ID3D11Buffer*> buffers;
		buffers.reserve( (int)VertexAttribute::TotalAttributes );
		std::vector<UINT> strides;
		strides.reserve( (int)VertexAttribute::TotalAttributes );

		if( pPipeline->RequiresVertexAttribute( VertexAttribute::Position ) )
		{
			ASSERT( m_bufPosition.GetVertexCount() > 0, "Shader requires model to have vertex position data" );
			buffers.emplace_back( m_bufPosition );
			strides.emplace_back( m_bufPosition.GetStride() );
		}
		if( pPipeline->RequiresVertexAttribute( VertexAttribute::Colour ) )
		{
			ASSERT( m_bufColour.GetVertexCount() > 0, "Shader requires model to have vertex colour data" );
			buffers.emplace_back( m_bufColour );
			strides.emplace_back( m_bufColour.GetStride() );
		}
		if( pPipeline->RequiresVertexAttribute( VertexAttribute::Normal ) )
		{
			ASSERT( m_bufNormal.GetVertexCount() > 0, "Shader requires model to have vertex normal data" );
			buffers.emplace_back( m_bufNormal );
			strides.emplace_back( m_bufNormal.GetStride() );
		}
		if( pPipeline->RequiresVertexAttribute( VertexAttribute::UV ) )
		{
			ASSERT( m_bufUV.GetVertexCount() > 0, "Shader requires model to have vertex UV data" );
			buffers.emplace_back( m_bufUV );
			strides.emplace_back( m_bufUV.GetStride() );
		}
		if( pPipeline->RequiresVertexAttribute( VertexAttribute::Tangent ) )
		{
			ASSERT( m_bufTangent.GetVertexCount() > 0, "Shader requires model to have vertex tangent data" );
			buffers.emplace_back( m_bufTangent );
			strides.emplace_back( m_bufTangent.GetStride() );
		}
		if( pPipeline->RequiresVertexAttribute( VertexAttribute::Bitangent ) )
		{
			ASSERT( m_bufTangent.GetVertexCount() > 0, "Shader requires model to have vertex bitangent data" );
			buffers.emplace_back( m_bufBitangent );
			strides.emplace_back( m_bufBitangent.GetStride() );
		}

		std::vector<UINT> offsets( buffers.size() );
		RenderContext::GetDeviceContext()->IASetVertexBuffers( 0, (UINT)buffers.size(), buffers.data(), strides.data(), offsets.data() );
		m_bufIndices.Bind();
	}

	void Mesh::SetData( const MeshParameters& params )
	{
		if( !params.position.empty() )
		{
			m_bufPosition.SetData( params.position );
		}
		if( !params.colour.empty() )
		{
			m_bufColour.SetData( params.colour );
		}
		if( !params.normal.empty() )
		{
			m_bufNormal.SetData( params.normal );
		}
		if( !params.uv.empty() )
		{
			m_bufUV.SetData( params.uv );
		}
		if( !params.tangent.empty() )
		{
			m_bufTangent.SetData( params.tangent );
		}
		if( !params.bitangent.empty() )
		{
			m_bufBitangent.SetData( params.bitangent );
		}
	}

	void Mesh::SetIndices( const std::vector<int>& indices )
	{
		m_bufIndices.SetData( indices );
	}

	Material& Mesh::GetMaterial()
	{
		return m_Material;
	}

	size_t Mesh::GetVertexCount() const
	{
		return m_bufPosition.GetVertexCount();
	}

	size_t Mesh::GetIndexCount() const
	{
		return m_bufIndices.GetIndexCount();
	}
}