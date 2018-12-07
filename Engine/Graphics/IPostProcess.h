#pragma once

struct ID3D11ShaderResourceView;

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