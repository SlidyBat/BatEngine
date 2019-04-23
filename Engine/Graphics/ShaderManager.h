#pragma once

#include "IPipeline.h"

namespace Bat
{
	enum
	{
		PS_CBUF_SLOT_0 = 1,
		PS_CBUF_SLOT_1,
		PS_CBUF_SLOT_2,
		PS_CBUF_SLOT_3,
		PS_CBUF_SLOT_4,
		PS_CBUF_SLOT_5,
		PS_CBUF_SLOT_6,
		PS_CBUF_SLOT_7
	};

	enum
	{
		PS_SAMP_SLOT_0 = 2,
		PS_SAMP_SLOT_1,
		PS_SAMP_SLOT_2,
		PS_SAMP_SLOT_3,
		PS_SAMP_SLOT_4,
		PS_SAMP_SLOT_5,
		PS_SAMP_SLOT_6,
		PS_SAMP_SLOT_7,
	};

	enum
	{
		PS_TEX_SLOT_0 = 0,
		PS_TEX_SLOT_1,
		PS_TEX_SLOT_2,
		PS_TEX_SLOT_3,
		PS_TEX_SLOT_4,
		PS_TEX_SLOT_5,
		PS_TEX_SLOT_6,
		PS_TEX_SLOT_7,
	};

	class ShaderManager
	{
	public:
		static void BindShaderGlobals( const class Camera* pCamera, const Vec2& resolution, IGPUContext* pContext );
		static IPipeline* GetPipeline( const std::string& name );
		static void AddPipeline( const std::string& name, std::unique_ptr<IPipeline> pPipeline );
	};
}
