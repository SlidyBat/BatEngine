#pragma once

#include "IGPUDevice.h"

namespace Bat
{
	class RenderTarget
	{
	public:
		RenderTarget( size_t width, size_t height, TexFormat format );
	private:
		std::unique_ptr<IRenderTarget> m_pRenderTarget = nullptr;
	};
}