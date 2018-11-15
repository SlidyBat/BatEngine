#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

namespace Bat
{
	struct ColourVertex
	{
		ColourVertex( const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT4& col )
			:
			position( pos ),
			colour( col )
		{}
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 colour;

		static constexpr int Inputs = 2;
		static constexpr D3D11_INPUT_ELEMENT_DESC InputLayout[Inputs] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
	};

	struct TexVertex
	{
		TexVertex( const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT2& tex )
			:
			position( pos ),
			texcoord( tex )
		{}
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texcoord;

		static constexpr int Inputs = 2;
		static constexpr D3D11_INPUT_ELEMENT_DESC InputLayout[Inputs] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
	};

	struct Vertex
	{
		Vertex( const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT2& tex )
			:
			position( pos ),
			normal( normal ),
			texcoord( tex )
		{}
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texcoord;

		static constexpr int Inputs = 3;
		static constexpr D3D11_INPUT_ELEMENT_DESC InputLayout[Inputs] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
	};
}