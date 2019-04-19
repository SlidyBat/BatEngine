#pragma once

#include "PCH.h"
#include "Scene.h"

namespace Bat
{
	class SceneLoader
	{
	public:
		static std::unique_ptr<ISceneNode> LoadScene( const std::string& filename );
	};
}