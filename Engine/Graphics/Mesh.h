#pragma once

#include "PCH.h"

#include "MathLib.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexTypes.h"
#include "IPipeline.h"
#include "Material.h"
#include "MeshAnimation.h"

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
		std::vector<Veu4> bone_ids;
		std::vector<Vec4> bone_weights;
	};

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh( const MeshParameters& params, const std::vector<unsigned int>& indices, const Material& material = {} );

		void SetName( const std::string& name ) { m_szName = name; }
		std::string GetName() const { return m_szName; }

		void Bind( IGPUContext* pContext, IVertexShader* pVertexShader ) const;

		void SetData( const MeshParameters& params );
		void SetIndices( const std::vector<unsigned int>& indices );

		Material& GetMaterial();
		const Material& GetMaterial() const;

		const Vec3* GetVertexData() const;
		size_t GetVertexCount() const;
		const unsigned int* GetIndexData() const;
		size_t GetIndexCount() const;

		bool HasTangentsAndBitangents() const { return m_bufTangent && m_bufTangent->GetVertexCount() > 0; }
		bool HasBones() const { return m_bufBoneIds && m_bufBoneIds->GetVertexCount() > 0; }

		const AABB& GetAABB() const { return m_Aabb; }
	private:
		VertexBuffer<Vec3> m_bufPosition;
		VertexBuffer<Vec4> m_bufColour;
		VertexBuffer<Vec3> m_bufNormal;
		VertexBuffer<Vec2> m_bufUV;
		VertexBuffer<Vec3> m_bufTangent;
		VertexBuffer<Vec3> m_bufBitangent;
		VertexBuffer<Veu4> m_bufBoneIds;
		VertexBuffer<Vec4> m_bufBoneWeights;
		IndexBuffer m_bufIndices;
		Material m_Material;
		AABB m_Aabb;

		std::vector<Vec3> m_vecPositions;
		std::vector<unsigned int> m_iIndices;
		std::vector<BoneData> m_Bones;

		std::string m_szName;
	};
}