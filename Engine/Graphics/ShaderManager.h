#pragma once

#include "IPipeline.h"
#include "Camera.h"

namespace Bat
{
	class Camera;
	class Mesh;

	enum
	{
		PS_CBUF_GLOBALS = 0,
		PS_CBUF_PARTICLES = 1,
		PS_CBUF_SHADOWMATRICES = 2,
		PS_CBUF_SLOT_0 = 3,
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
		PS_SAMP_WRAP = 0,
		PS_SAMP_CLAMP = 1,
		PS_SAMP_MIRROR = 2,
		PS_SAMP_BORDER = 4,
		PS_SAMP_CMP_DEPTH = 5,
		PS_SAMP_POINT = 6,
		PS_SAMP_LINEAR = 7,
		PS_SAMP_SLOT_0 = 8,
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
		PS_TEX_SHADOWMAPS = 0,
		PS_TEX_SLOT_0 = 1,
		PS_TEX_SLOT_1,
		PS_TEX_SLOT_2,
		PS_TEX_SLOT_3,
		PS_TEX_SLOT_4,
		PS_TEX_SLOT_5,
		PS_TEX_SLOT_6,
		PS_TEX_SLOT_7,
	};

	enum
	{
		VS_CBUF_TRANSFORMS = 0,
		VS_CBUF_BONES = 1,
		VS_CBUF_PARTICLES = 2,
		VS_CBUF_SLOT_0 = 3,
		VS_CBUF_SLOT_1,
		VS_CBUF_SLOT_2,
		VS_CBUF_SLOT_3,
		VS_CBUF_SLOT_4,
		VS_CBUF_SLOT_5,
		VS_CBUF_SLOT_6,
		VS_CBUF_SLOT_7
	};

	class ShaderManager
	{
	public:
		static void BindShaderGlobals( const Camera* pCamera, const Vec2& resolution, IGPUContext* pContext );
		
		template <typename Pipeline>
		static Pipeline* GetPipeline()
		{
			static_assert(std::is_convertible<Pipeline*, IPipeline*>::value, "Pipeline must inherit from IPipeline");

			auto type = std::type_index(typeid( Pipeline ));

			auto it = m_mapPipelines.find( type );
			if( it == m_mapPipelines.end() )
			{
				auto pPipeline = new Pipeline();
				m_mapPipelines[type] = std::unique_ptr<Pipeline>( pPipeline );
				return pPipeline;
			}

			return static_cast<Pipeline*>( it->second.get() );
		}
		
		static std::vector<ShaderMacro> BuildMacrosForMesh( const Mesh& mesh );
		static std::vector<ShaderMacro> BuildMacrosForInstancedMesh( const Mesh& mesh );
	private:
		static std::unordered_map<std::type_index, std::unique_ptr<IPipeline>> m_mapPipelines;
	};
}
