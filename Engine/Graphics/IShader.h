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

		virtual void Render( ID3D11DeviceContext* pDeviceContext, IShaderParameters* pParameters ) = 0;
	};
}