#pragma once

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexTypes.h"
#include <vector>

namespace Bat
{
	class Texture;

	template <typename T>
	class _Mesh
	{
	public:
		_Mesh( const std::vector<T>& vertices, const std::vector<int>& indices, Texture* pTexture )
			:
			vertices(vertices),
			indices(indices),
			m_pTexture(pTexture),
			m_VertexBuffer(vertices),
			m_IndexBuffer(indices)
		{}

		void Bind() const
		{
			g_pGfx->GetDeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			m_VertexBuffer.Bind();
			m_IndexBuffer.Bind();
		}

		Texture* GetTexture() const
		{
			return m_pTexture;
		}
		size_t GetVertexCount() const
		{
			return vertices.size();
		}
		size_t GetIndexCount() const
		{
			return indices.size();
		}
	public:
		std::vector<T> vertices;
		std::vector<int> indices;
	private:
		VertexBuffer<T> m_VertexBuffer;
		IndexBuffer m_IndexBuffer;
		Texture* m_pTexture;
	};

	using ColourMesh = _Mesh<ColourVertex>;
	using TexMesh = _Mesh<TexVertex>;
	using Mesh = _Mesh<Vertex>;
}