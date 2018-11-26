#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

namespace Bat
{
	enum class VertexAttribute
	{
		Invalid = -1,
		Position,
		Colour,
		Normal,
		UV,
		Tangent,
		TotalAttributes
	};

	class AttributeInfo
	{
	public:
		static VertexAttribute SemanticToAttribute( const std::string& semantic )
		{
			static std::unordered_map<std::string, VertexAttribute> m_mapConvert;
			static bool initialized = false;
			if( !initialized )
			{
				initialized = true;
				m_mapConvert["POSITION"] = VertexAttribute::Position;
				m_mapConvert["COLOR"] = VertexAttribute::Colour;
				m_mapConvert["NORMAL"] = VertexAttribute::Normal;
				m_mapConvert["TEXCOORD"] = VertexAttribute::UV;
				m_mapConvert["TANGENT"] = VertexAttribute::Tangent;
			}

			auto it = m_mapConvert.find( semantic );
			if( it == m_mapConvert.end() )
			{
				return VertexAttribute::Invalid;
			}

			return it->second;
		}

		static std::string AttributeToSemantic( const VertexAttribute attribute )
		{
			static std::unordered_map<VertexAttribute, std::string> m_mapConvert;
			static bool initialized = false;
			if( !initialized )
			{
				initialized = true;
				m_mapConvert[VertexAttribute::Position] = "POSITION";
				m_mapConvert[VertexAttribute::Colour] = "COLOR";
				m_mapConvert[VertexAttribute::Normal] = "NORMAL";
				m_mapConvert[VertexAttribute::UV] = "TEXCOORD";
				m_mapConvert[VertexAttribute::Tangent] = "TANGENT";
			}

			auto it = m_mapConvert.find( attribute );
			if( it == m_mapConvert.end() )
			{
				return "";
			}

			return it->second;
		}
	};

	struct ColourVertex
	{
		ColourVertex() = default;
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
		TexVertex() = default;
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
		Vertex() = default;
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
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
	};
}