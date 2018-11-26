#pragma once

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexTypes.h"
#include "IPipeline.h"
#include <vector>
#include "BatAssert.h"

namespace Bat
{
	class Texture;
	class Material;

	struct MeshParameters
	{
		std::vector<Vec4> position;
		std::vector<Vec4> colour;
		std::vector<Vec4> normal;
		std::vector<Vec2> uv;
		std::vector<Vec4> tangent;
	};

	class Mesh
	{
	public:
		Mesh( const MeshParameters& params, const std::vector<int>& indices, Material* pMaterial )
			:
			m_bufIndices( indices ),
			m_pMaterial( pMaterial )
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
		}

		void Bind( IPipeline* pPipeline ) const
		{
			// TODO: support other primitives?
			g_pGfx->GetDeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			// bind buffers
			UINT curslot = 0;
			if( pPipeline->RequiresVertexAttribute( VertexAttribute::Position ) )
			{
				ASSERT( m_bufPosition.GetVertexCount() > 0, "Shader requires model to have vertex position data" );
				m_bufPosition.Bind( curslot++ );
			}
			if( pPipeline->RequiresVertexAttribute( VertexAttribute::Colour ) )
			{
				ASSERT( m_bufColour.GetVertexCount() > 0, "Shader requires model to have vertex colour data" );
				m_bufColour.Bind( curslot++ );
			}
			if( pPipeline->RequiresVertexAttribute( VertexAttribute::Normal ) )
			{
				ASSERT( m_bufNormal.GetVertexCount() > 0, "Shader requires model to have vertex normal data" );
				m_bufNormal.Bind( curslot++ );
			}
			if( pPipeline->RequiresVertexAttribute( VertexAttribute::UV ) )
			{
				ASSERT( m_bufUV.GetVertexCount() > 0, "Shader requires model to have vertex UV data" );
				m_bufUV.Bind( curslot++ );
			}
			if( pPipeline->RequiresVertexAttribute( VertexAttribute::Tangent ) )
			{
				ASSERT( m_bufTangent.GetVertexCount() > 0, "Shader requires model to have vertex tangent data" );
				m_bufTangent.Bind( curslot++ );
			}

			m_bufIndices.Bind();
		}

		Material* GetMaterial() const
		{
			return m_pMaterial;
		}
		size_t GetVertexCount() const
		{
			return m_bufPosition.GetVertexCount();
		}
		size_t GetIndexCount() const
		{
			return m_bufIndices.GetIndexCount();
		}
	private:
		VertexBuffer<Vec4> m_bufPosition;
		VertexBuffer<Vec4> m_bufColour;
		VertexBuffer<Vec4> m_bufNormal;
		VertexBuffer<Vec2> m_bufUV;
		VertexBuffer<Vec4> m_bufTangent;
		IndexBuffer m_bufIndices;
		Material* m_pMaterial;
	};
}