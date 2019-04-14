#include "PCH.h"
#include "RenderTarget.h"

#include "Globals.h"

namespace Bat
{
	RenderTarget::RenderTarget( size_t width, size_t height, TexFormat format )
		:
		m_pRenderTarget( gpu->CreateRenderTarget( width, height, format ) )
	{}
}