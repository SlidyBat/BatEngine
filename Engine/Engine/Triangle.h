#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "Vertex.h"
#include <array>

class Triangle
{
public:
	Triangle( ID3D11Device* pDevice, const std::array<Vertex, 3>& points );
	Triangle( ID3D11Device* pDevice, Vertex v1, Vertex v2, Vertex v3 )
		:
		Triangle( pDevice, std::array<Vertex, 3>{ v1, v2, v3 } )
	{}
	~Triangle();
	Triangle( const Triangle& src ) = delete;
	Triangle& operator=( const Triangle& src ) = delete;
	Triangle( Triangle&& donor ) = delete;
	Triangle& operator=( Triangle&& donor ) = delete;

	void Render( ID3D11DeviceContext* pDeviceContext );
private:
	void RenderBuffers( ID3D11DeviceContext* pDeviceContext );
private:
	ID3D11Buffer*			m_pVertexBuffer = nullptr;

	std::array<Vertex, 3>	triangleVerts;
};