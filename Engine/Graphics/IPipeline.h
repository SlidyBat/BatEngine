#pragma once

#include "VertexShader.h"
#include "PixelShader.h"
#include <DirectXMath.h>

namespace Bat
{
	class IPipelineParameters
	{
	public:
		virtual ~IPipelineParameters() = default;
	};

	class IPipeline
	{
	public:
		virtual ~IPipeline() = default;

		virtual void BindParameters( IPipelineParameters* pParameters ) = 0;
		virtual void Render( UINT vertexcount ) = 0;
		virtual void RenderIndexed( UINT indexcount ) = 0;
	};
}