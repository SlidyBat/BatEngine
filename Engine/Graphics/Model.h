#pragma once

#include "PCH.h"

#include "Camera.h"
#include "Texture.h"
#include "VertexTypes.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ResourceManager.h"

namespace Bat
{
	class IPipeline;

	class ModelComponent : public Component<ModelComponent>
	{
	public:
		ModelComponent( std::vector<Resource<Mesh>> pMeshes );

		std::vector<Resource<Mesh>>& GetMeshes();
		const std::vector<Resource<Mesh>>& GetMeshes() const;

		float GetScale() const;
		void SetScale( const float scale );

		// Returns mins of model in object space
		const Vec3& GetMins() const;
		// Returns maxs of model in object space
		const Vec3& GetMaxs() const;
	protected:
		Vec3 m_vecMins;
		Vec3 m_vecMaxs;

		std::vector<Resource<Mesh>> m_pMeshes;
	};
}