#pragma once

#include "Graphics/Mesh.h"

namespace Bat
{
	using NavMeshHandle_t = size_t;

	class NavMesh;

	class NavMeshSystem
	{
	public:
		NavMeshSystem();
		~NavMeshSystem();
		NavMeshHandle_t Bake();

		std::vector<Vec3> GetPath( NavMeshHandle_t navmesh, const Vec3& start, const Vec3& end ) const;
		void Draw( NavMeshHandle_t navmesh ) const;
	private:
		std::vector<NavMesh> m_NavMeshes;
	};
}