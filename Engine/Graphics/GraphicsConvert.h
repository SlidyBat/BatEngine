#pragma once

#include "Texture.h"

namespace Bat
{
	class GraphicsConvert
	{
	public:
		static IRenderTarget* EquirectangularToCubemap( IGPUContext* pContext, ITexture* equirect, size_t width, size_t height );
	};
}