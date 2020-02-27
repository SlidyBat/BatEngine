#pragma once

#include "Mesh.h"

namespace Bat
{
	using NavMeshHandle_t = size_t;

	class NavMesh;

	struct NavMeshAgent : public Component<NavMeshAgent>
	{
		Vec3 target = { 0.0f, 0.0f, 0.0f };
		bool go_to_target = false;
		std::vector<Vec3> path;
		size_t next_path_point = 0;
		NavMeshHandle_t navmesh;
	};

	class NavMeshSystem
	{
	public:
		NavMeshSystem();
		~NavMeshSystem();
		NavMeshHandle_t Bake();

		void Update( float dt );
	private:
		std::vector<NavMesh> m_NavMeshes;
	};
}