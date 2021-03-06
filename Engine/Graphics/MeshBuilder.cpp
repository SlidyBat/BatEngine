#include "PCH.h"
#include "MeshBuilder.h"

namespace Bat
{
	MeshBuilder::MeshBuilder( size_t triangle_estimate, PrimitiveTopology topology )
		:
		MeshBuilder( triangle_estimate * 3, triangle_estimate * 3, topology )
	{}
	MeshBuilder::MeshBuilder( size_t vertices_estimate, size_t indices_estimate, PrimitiveTopology topology )
		:
		topology( topology )
	{
		if( vertices_estimate )
		{
			mesh_params.position.reserve( vertices_estimate );
		}
		if( indices_estimate )
		{
			indices.reserve( indices_estimate );
		}
	}
	void MeshBuilder::Position( float x, float y, float z )
	{
		mesh_params.position.emplace_back( x, y, z );
	}
	void MeshBuilder::Position( const Vec3& pos )
	{
		mesh_params.position.emplace_back( pos );
	}
	void MeshBuilder::Position( const Vec2& pos )
	{
		Position( pos.x, pos.y );
	}
	void MeshBuilder::Colour( float r, float g, float b, float a )
	{
		mesh_params.colour.emplace_back( r, g, b, a );
	}
	void MeshBuilder::Colour( const Vec4& col )
	{
		mesh_params.colour.emplace_back( col );
	}
	void MeshBuilder::Index( unsigned int index )
	{
		indices.push_back( index );
	}
	void MeshBuilder::Triangle( unsigned int index1, unsigned int index2, unsigned int index3 )
	{
		Index( index1 );
		Index( index2 );
		Index( index3 );
	}
	void MeshBuilder::Line( unsigned int index1, unsigned int index2 )
	{
		Index( index1 );
		Index( index2 );
	}
	Mesh MeshBuilder::Build()
	{
		Mesh mesh( mesh_params );
		if( !indices.empty() )
		{
			mesh.SetIndices( indices );
		}
		mesh.SetTopology( topology );

		return mesh;
	}
}
