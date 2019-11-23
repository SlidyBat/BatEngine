#pragma once

#include "IGPUDevice.h"

namespace Bat
{
	class ScratchRenderTarget
	{
	public:
		static ScratchRenderTarget Create( size_t width, size_t height, TexFormat format );
		static void Clear();

		~ScratchRenderTarget();

		ScratchRenderTarget( const ScratchRenderTarget& ) = delete;
		ScratchRenderTarget& operator=( const ScratchRenderTarget& ) = delete;
		ScratchRenderTarget( ScratchRenderTarget&& ) = delete;
		ScratchRenderTarget& operator=( ScratchRenderTarget&& ) = delete;

		operator IRenderTarget* ();
		IRenderTarget* operator->();
	private:
		ScratchRenderTarget( IRenderTarget* rt );
	private:
		IRenderTarget* m_pRenderTarget;
	};
}