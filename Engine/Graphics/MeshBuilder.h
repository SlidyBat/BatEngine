#pragma once

#include "Mesh.h"

namespace Bat
{
	class MeshBuilder
	{
	public:
		MeshBuilder( size_t triangle_estimate = 0, PrimitiveTopology topology = PrimitiveTopology::TRIANGLELIST );
		MeshBuilder( size_t vertices_estimate, size_t indices_estimate, PrimitiveTopology topology = PrimitiveTopology::TRIANGLELIST );
	
		void Position( float x, float y, float z = 100.0f );
		void Position( const Vec3& pos );
		void Position( const Vec2& pos );
		void Colour( float r, float g, float b, float a = 1.0f );
		void Colour( const Vec4& col );

		void Index( unsigned int index );
		void Triangle( unsigned int index1, unsigned int index2, unsigned int index3 );
		void Line( unsigned int index1, unsigned int index2 );

		Mesh Build();
	private:
		MeshParameters mesh_params;
		std::vector<unsigned int> indices;
		PrimitiveTopology topology;
	};
}