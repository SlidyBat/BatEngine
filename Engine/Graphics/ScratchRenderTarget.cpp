#include "PCH.h"
#include "ScratchRenderTarget.h"

namespace Bat
{
	struct TrackedRenderTarget
	{
		std::unique_ptr<IRenderTarget> render_target;
		bool in_use = false;
	};

	static std::vector<TrackedRenderTarget> g_TrackedRenderTargets;

	ScratchRenderTarget ScratchRenderTarget::Create( size_t width, size_t height, TexFormat format )
	{
		for( TrackedRenderTarget& trt : g_TrackedRenderTargets )
		{
			if( !trt.in_use &&
				trt.render_target->GetWidth() == width &&
				trt.render_target->GetHeight() == height &&
				trt.render_target->GetFormat() == format )
			{
				trt.in_use = true;
				return ScratchRenderTarget( trt.render_target.get() );
			}
		}

		TrackedRenderTarget new_trt;
		new_trt.in_use = true;
		new_trt.render_target = std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( width, height, format ) );
		auto rt = new_trt.render_target.get();

		g_TrackedRenderTargets.push_back( std::move( new_trt ) );

		return ScratchRenderTarget( rt );
	}

	void ScratchRenderTarget::Clear()
	{
		g_TrackedRenderTargets.clear();
	}

	ScratchRenderTarget::~ScratchRenderTarget()
	{
		for( auto it = g_TrackedRenderTargets.begin(); it != g_TrackedRenderTargets.end(); ++it )
		{
			TrackedRenderTarget& trt = *it;
			if( m_pRenderTarget == trt.render_target.get() )
			{
				trt.in_use = false;
				break;
			}
		}
	}
	ScratchRenderTarget::operator IRenderTarget* ()
	{
		return m_pRenderTarget;
	}
	IRenderTarget* ScratchRenderTarget::operator->()
	{
		return m_pRenderTarget;
	}
	ScratchRenderTarget::ScratchRenderTarget( IRenderTarget* rt )
		:
		m_pRenderTarget( rt )
	{}
}
