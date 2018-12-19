#pragma once

#include "PCH.h"
#include "Scene.h"

namespace Bat
{
	class SceneLoader
	{
	public:
		static SceneGraph LoadScene( const std::string& filename );
	};
}