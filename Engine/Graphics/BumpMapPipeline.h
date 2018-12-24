#pragma once

#include "LightPipeline.h"

namespace Bat
{
	class BumpMapPipeline : public LightPipeline
	{
	public:
		BumpMapPipeline( const std::string& vsFilename, const std::string& psFilename );

		void BindParameters( IPipelineParameters& pParameters ) override;
	};
}