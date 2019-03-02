#pragma once

#include "IPipeline.h"

namespace Bat
{
	class ShaderManager
	{
	public:
		static IPipeline* GetPipeline( const std::string& name );
		static void AddPipeline( const std::string& name, std::unique_ptr<IPipeline> pPipeline );
	};
}
