#pragma once

#include <d3d11.h>

namespace Bat
{
	class RenderTexture;

	class IPostProcess
	{
	public:
		virtual ~IPostProcess() = default;

		virtual void Render( ID3D11ShaderResourceView* pTexture ) = 0;
	};
}