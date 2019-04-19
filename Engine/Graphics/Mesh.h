#pragma once

#include "PCH.h"

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexTypes.h"
#include "IPipeline.h"
#include "Material.h"

namespace Bat
{
	class Texture;
	class Material;

	struct MeshParameters
	{
		std::vector<Vec3> position;
		std::vector<Vec4> colour;
		std::vector<Vec3> normal;
		std::vector<Vec2> uv;
		std::vector<Vec3> tangent;
		std::vector<Vec3> bitangent;
	};

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh( const MeshParameters& params, const std::vector<unsigned int>& indices, const Material& material = {} );

		void SetName( const std::string& name ) { m_szName = name; }
		std::string GetName() const { return m_szName; }

		void Bind( IGPUContext* pContext, IPipeline* pPipeline ) const;

		void SetData( const MeshParameters& params );
		void SetIndices( const std::vector<unsigned int>& indices );

		Material& GetMaterial();

		size_t GetVertexCount() const;
		size_t GetIndexCount() const;
	private:
		VertexBuffer<Vec3> m_bufPosition;
		VertexBuffer<Vec4> m_bufColour;
		VertexBuffer<Vec3> m_bufNormal;
		VertexBuffer<Vec2> m_bufUV;
		VertexBuffer<Vec3> m_bufTangent;
		VertexBuffer<Vec3> m_bufBitangent;
		IndexBuffer m_bufIndices;
		Material m_Material;

		std::string m_szName;
	};
}