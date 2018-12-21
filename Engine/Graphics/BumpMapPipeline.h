#pragma once

#include "LightPipeline.h"

namespace Bat
{
	class BumpMapPipeline : public LightPipeline
	{
	public:
		BumpMapPipeline( const std::wstring& vsFilename, const std::wstring& psFilename );

		void BindParameters( IPipelineParameters& pParameters ) override;
	};
}