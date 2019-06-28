#pragma once

#include "PCH.h"
#include "Entity.h"

namespace Bat
{
	class SceneLoader
	{
	public:
		static SceneNode LoadScene( const std::string& filename );
	};
}