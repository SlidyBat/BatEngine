#pragma once

#include "VertexShader.h"
#include "PixelShader.h"
#include <DirectXMath.h>

namespace Bat
{
	class IShaderParameters
	{
	public:
		virtual ~IShaderParameters() = default;
	};

	class IShader
	{
	public:
		virtual ~IShader() = default;

		virtual void BindParameters( IShaderParameters* pParameters ) = 0;
		virtual void Render( UINT vertexcount ) = 0;
		virtual void RenderIndexed( UINT indexcount ) = 0;
	};
}