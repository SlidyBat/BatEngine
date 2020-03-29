#pragma once

#include "PCH.h"

#include "Camera.h"
#include "Texture.h"
#include "VertexTypes.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ResourceManager.h"
#include "RenderFlags.h"

namespace Bat
{
	class IPipeline;

	class ModelComponent
	{
	public:
		BAT_COMPONENT( MODEL );

		ModelComponent( std::vector<Resource<Mesh>> pMeshes );

		std::vector<Resource<Mesh>>& GetMeshes();
		const std::vector<Resource<Mesh>>& GetMeshes() const;

		// Gets AABB in model space
		const AABB& GetAABB() const { return m_Aabb; }

		bool HasRenderFlag( RenderFlags flag ) { return (m_RenderFlags & flag) == flag; }
		void AddRenderFlag( RenderFlags flag ) { m_RenderFlags |= flag; }
		void RemoveRenderFlag( RenderFlags flag ) { m_RenderFlags &= ~flag; }

		void DoImGuiMenu();
	protected:
		AABB m_Aabb;

		std::vector<Resource<Mesh>> m_pMeshes;

		RenderFlags m_RenderFlags = RenderFlags::NONE;
	};
}