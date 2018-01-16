#pragma once

#include <DirectXMath.h>

struct Vertex
{
	Vertex( const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT4& col )
		:
		position( pos ),
		colour( col )
	{}
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 colour;
};