#pragma once

#include "Texture.h"

namespace Bat
{
	class GraphicsConvert
	{
	public:
		static ITexture* EquirectangularToCubemap( IGPUContext* pContext, ITexture* equirect, size_t width, size_t height );

		static ITexture* MakeIrradianceMap( IGPUContext* pContext, ITexture* envmap, size_t width, size_t height );
		static ITexture* MakePreFilteredEnvMap( IGPUContext* pContext, ITexture* envmap, size_t width, size_t height );
		static ITexture* MakeBrdfIntegrationMap( IGPUContext* pContext, size_t width, size_t height );
	};
}