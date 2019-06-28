#pragma once

#include "MathLib.h"
#include "Entity.h"
#include "Model.h"

namespace Bat
{
	struct NameComponent : public Component<NameComponent>
	{
		NameComponent( std::string name )
			:
			name( std::move( name ) )
		{}

		std::string name;
	};

	struct TransformComponent : public Component<TransformComponent>
	{
		TransformComponent( DirectX::XMMATRIX transform )
			:
			transform( transform )
		{}

		DirectX::XMMATRIX transform;
	};

	struct ModelComponent : public Component<ModelComponent>
	{
		ModelComponent( const std::vector<Resource<Mesh>>& meshes )
			:
			model( meshes )
		{}

		Model model;
	};
}