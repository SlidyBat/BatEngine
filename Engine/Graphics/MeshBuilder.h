#pragma once

#include "Mesh.h"

namespace Bat
{
	class MeshBuilder
	{
	public:
		MeshBuilder( size_t triangle_estimate = 0 );
	
		void Position( float x, float y, float z = 100.0f );
		void Colour( float r, float g, float b, float a = 1.0f );
		void Colour( const Vec4& col );

		void Index( unsigned int index );
		void Triangle( unsigned int index1, unsigned int index2, unsigned int index3 );

		Mesh Build();
	private:
		MeshParameters mesh_params;
		std::vector<unsigned int> indices;
	};
}