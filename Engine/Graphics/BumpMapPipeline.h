#pragma once

#include "LightPipeline.h"

namespace Bat
{
	class BumpMapPipeline : public LightPipeline
	{
	public:
		BumpMapPipeline( const std::string& vs_filename, const std::string& ps_filename );

		void BindParameters( IGPUContext* pContext, IPipelineParameters& pParameters ) override;
	private:
		ConstantBuffer<CB_LightPipelineMatrix> m_cbufTransform;
		ConstantBuffer<CB_LightPipelineLightingParams> m_cbufLightParams;
		ConstantBuffer<CB_LightPipelineLight> m_cbufLight;
	};
}