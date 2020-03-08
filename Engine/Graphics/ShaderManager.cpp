#include "PCH.h"
#include "ShaderManager.h"

#include "Globals.h"
#include "ConstantBuffer.h"
#include "Mesh.h"

namespace Bat
{
	std::unordered_map<std::type_index, std::unique_ptr<IPipeline>> ShaderManager::m_mapPipelines;

	struct PSGlobals
	{
		// width/height of screen
		Vec2 resolution;
		float time;
		float deltatime;
		Vec3 camera_pos;
		float pad;
		Mat4 camera_inv_vp;
	};

	void ShaderManager::BindShaderGlobals( const Camera* pCamera, const Vec2& resolution, IGPUContext* pContext )
	{
		static ConstantBuffer<PSGlobals> cb_globals;
		static std::vector<std::unique_ptr<ISampler>> samplers;

		static bool initialized = false;
		if( !initialized )
		{
			// Initialize shader globals
			SamplerDesc sampler_desc;
			sampler_desc.filter = SampleFilter::ANISOTROPIC;
			sampler_desc.mip_lod_bias = -0.5f;
			sampler_desc.max_anisotropy = 8;
			sampler_desc.comparison_func = ComparisonFunc::ALWAYS;
			sampler_desc.border_color[0] = 1.0f;
			sampler_desc.border_color[1] = 1.0f;
			sampler_desc.border_color[2] = 1.0f;
			sampler_desc.border_color[3] = 1.0f;
			sampler_desc.min_lod = 0;
			sampler_desc.max_lod = FLT_MAX;

			// wrap sampler
			sampler_desc.address_u = TextureAddressMode::WRAP;
			sampler_desc.address_v = TextureAddressMode::WRAP;
			sampler_desc.address_w = TextureAddressMode::WRAP;
			samplers.emplace_back( gpu->CreateSampler( sampler_desc ) );

			// clamp sampler
			sampler_desc.address_u = TextureAddressMode::CLAMP;
			sampler_desc.address_v = TextureAddressMode::CLAMP;
			sampler_desc.address_w = TextureAddressMode::CLAMP;
			samplers.emplace_back( gpu->CreateSampler( sampler_desc ) );

			// mirror sampler
			sampler_desc.address_u = TextureAddressMode::MIRROR;
			sampler_desc.address_v = TextureAddressMode::MIRROR;
			sampler_desc.address_w = TextureAddressMode::MIRROR;
			samplers.emplace_back( gpu->CreateSampler( sampler_desc ) );

			// border sampler
			sampler_desc.address_u = TextureAddressMode::BORDER;
			sampler_desc.address_v = TextureAddressMode::BORDER;
			sampler_desc.address_w = TextureAddressMode::BORDER;
			samplers.emplace_back( gpu->CreateSampler( sampler_desc ) );

			// compare depth sampler
			sampler_desc.filter = SampleFilter::COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			sampler_desc.address_u = TextureAddressMode::CLAMP;
			sampler_desc.address_v = TextureAddressMode::CLAMP;
			sampler_desc.address_w = TextureAddressMode::CLAMP;
			sampler_desc.mip_lod_bias = 0.0f;
			sampler_desc.max_anisotropy = 0;
			sampler_desc.comparison_func = ComparisonFunc::GREATER_EQUAL;
			samplers.emplace_back( gpu->CreateSampler( sampler_desc ) );

			// point sampler
			sampler_desc.filter = SampleFilter::MIN_MAG_MIP_POINT;
			sampler_desc.address_u = TextureAddressMode::WRAP;
			sampler_desc.address_v = TextureAddressMode::WRAP;
			sampler_desc.address_w = TextureAddressMode::WRAP;
			sampler_desc.mip_lod_bias = 0.0f;
			sampler_desc.max_anisotropy = 0;
			sampler_desc.comparison_func = ComparisonFunc::ALWAYS;
			samplers.emplace_back( gpu->CreateSampler( sampler_desc ) );

			// linear sampler
			sampler_desc.filter = SampleFilter::MIN_MAG_MIP_LINEAR;
			sampler_desc.address_u = TextureAddressMode::WRAP;
			sampler_desc.address_v = TextureAddressMode::WRAP;
			sampler_desc.address_w = TextureAddressMode::WRAP;
			sampler_desc.mip_lod_bias = 0.0f;
			sampler_desc.max_anisotropy = 0;
			sampler_desc.comparison_func = ComparisonFunc::ALWAYS;
			samplers.emplace_back( gpu->CreateSampler( sampler_desc ) );

			// linear sampler
			sampler_desc.address_u = TextureAddressMode::CLAMP;
			sampler_desc.address_v = TextureAddressMode::CLAMP;
			sampler_desc.address_w = TextureAddressMode::CLAMP;
			samplers.emplace_back( gpu->CreateSampler( sampler_desc ) );

			initialized = true;
		}

		// Bind shader globals
		for( size_t i = 0; i < samplers.size(); i++ )
		{
			gpu->GetContext()->SetSampler( ShaderType::PIXEL, samplers[i].get(), i );
		}

		PSGlobals g;
		g.resolution = resolution;
		g.time = g_pGlobals->elapsed_time;
		g.deltatime = g_pGlobals->deltatime;
		g.camera_pos = pCamera ? pCamera->GetPosition() : Vec3{ 0.0f, 0.0f, 0.0f };
		g.camera_inv_vp = pCamera ? Mat4::Inverse( pCamera->GetViewMatrix() * pCamera->GetProjectionMatrix() ) : Mat4::Identity();
		cb_globals.Update( pContext, g );
		pContext->SetConstantBuffer( ShaderType::PIXEL, cb_globals, PS_CBUF_GLOBALS );
	}

	static std::vector<ShaderMacro> BuildMacrosForAnyMesh( const Mesh& mesh )
	{
		std::vector<ShaderMacro> macros;
		if( mesh.HasTangentsAndBitangents() )
		{
			macros.emplace_back( "HAS_TANGENT" );
		}

		const Material& material = mesh.GetMaterial();

		if( material.GetAlphaMode() == AlphaMode::MASK )
		{
			macros.emplace_back( "MASK_ALPHA" );
		}

		return macros;
	}

	std::vector<ShaderMacro> ShaderManager::BuildMacrosForMesh( const Mesh& mesh )
	{
		std::vector<ShaderMacro> macros = BuildMacrosForAnyMesh( mesh );
		if( mesh.HasBones() )
		{
			macros.emplace_back( "HAS_BONES" );
		}

		return macros;
	}

	std::vector<ShaderMacro> ShaderManager::BuildMacrosForInstancedMesh( const Mesh& mesh )
	{
		std::vector<ShaderMacro> macros = BuildMacrosForAnyMesh( mesh );
		macros.emplace_back( "INSTANCED" );

		return macros;
	}
}
