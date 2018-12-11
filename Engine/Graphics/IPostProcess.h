#pragma once

struct ID3D11ShaderResourceView;

namespace Bat
{
	class RenderTexture;

	class IPostProcess
	{
	public:
		virtual ~IPostProcess() = default;

		virtual void Render( RenderTexture& v ) = 0;
	};
}