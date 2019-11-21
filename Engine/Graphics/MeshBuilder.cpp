#include "PCH.h"
#include "MeshBuilder.h"

namespace Bat
{
	MeshBuilder::MeshBuilder( size_t triangle_estimate )
	{
		if( triangle_estimate )
		{
			mesh_params.position.reserve( triangle_estimate );
			indices.reserve( triangle_estimate * 3 );
		}
	}
	void MeshBuilder::Position( float x, float y, float z )
	{
		mesh_params.position.emplace_back( x, y, z );
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
	Mesh MeshBuilder::Build()
	{
		return Mesh( mesh_params, indices );
	}
}
