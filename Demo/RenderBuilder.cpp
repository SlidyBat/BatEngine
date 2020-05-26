#include "RenderBuilder.h"

#include <BatEngine.h>

#include "imgui.h"

using namespace Bat;

void RenderBuilder::Make( const Bat::Renderer& gfx, const Bat::Window& wnd, Bat::RenderGraph* graph )
{
	MakeForwardPipeline( gfx, wnd, graph );
}

void RenderBuilder::MakeForwardPipeline( const Bat::Renderer& gfx, const Bat::Window& wnd, Bat::RenderGraph* graph )
{
	IGPUContext* pContext = gpu->GetContext();

	ScratchRenderTarget::Clear();
	ShaderManager::BindShaderGlobals( gfx.GetActiveCamera(), { (float)wnd.GetWidth(), (float)wnd.GetHeight() }, pContext );

	graph->Reset();

	int post_process_count = 0;
	if( bloom_enabled ) post_process_count++;
	if( motion_blur_enabled ) post_process_count++;
	if( tonemapping_enabled ) post_process_count++;

	size_t ms_samples = msaa_enabled ? (size_t)msaa_samples : 1;
	MsaaQuality ms_quality = msaa_enabled ? MsaaQuality::STANDARD_PATTERN : MsaaQuality::NONE;

	// initialize resources
	// render texture to draw scene to
	if( post_process_count )
	{
		if( msaa_enabled )
		{
			graph->AddRenderTargetResource( "target",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT, ms_quality, ms_samples ) ) );
			graph->AddRenderTargetResource( "target2",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT ) ) );
			graph->AddRenderTargetResource( "resolve_target",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT ) ) );
		}
		else
		{
			graph->AddRenderTargetResource( "target",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT, ms_quality, ms_samples ) ) );
			graph->AddRenderTargetResource( "target2",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT, ms_quality, ms_samples ) ) );
		}
	}
	else
	{
		if( msaa_enabled )
		{
			graph->AddRenderTargetResource( "target",
				std::unique_ptr<IRenderTarget>( gpu->CreateRenderTarget( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R32G32B32A32_FLOAT, ms_quality, ms_samples, TexFlags::NO_SHADER_BIND ) ) );
			graph->AddRenderTargetResource( "resolve_target", gpu->GetBackbuffer() );
		}
		else
		{
			graph->AddRenderTargetResource( "target", gpu->GetBackbuffer() );
		}
	}

	auto depth = std::unique_ptr<IDepthStencil>( gpu->CreateDepthStencil( wnd.GetWidth(), wnd.GetHeight(), TEX_FORMAT_R24G8_TYPELESS, 1, ms_quality, ms_samples, TexFlags::NO_SHADER_BIND ) );
	graph->AddDepthStencilResource( "depth", std::move( depth ) );

	std::unique_ptr<ITexture> envmap;
	if( Bat::GetFileExtension( skybox_tex ) == "hdr" )
	{
		auto hdrmap = std::unique_ptr<ITexture>( gpu->CreateTexture( skybox_tex, TexFlags::NO_GEN_MIPS ) );
		envmap = std::unique_ptr<ITexture>( GraphicsConvert::EquirectangularToCubemap( pContext, hdrmap.get(), 512, 512 ) );
	}
	else
	{
		envmap = std::unique_ptr<ITexture>( gpu->CreateTexture( skybox_tex, TexFlags::NO_GEN_MIPS ) );
	}
	auto irradiance = std::unique_ptr<ITexture>( GraphicsConvert::MakeIrradianceMap( pContext, envmap.get(), 32, 32 ) );
	auto prefilter = std::unique_ptr<ITexture>( GraphicsConvert::MakePreFilteredEnvMap( pContext, envmap.get(), 128, 128 ) );
	auto brdf_integration = std::unique_ptr<ITexture>( GraphicsConvert::MakeBrdfIntegrationMap( pContext, 512, 512 ) );

	graph->AddTextureResource( "skybox", std::move( envmap ) );
	graph->AddTextureResource( "irradiance", std::move( irradiance ) );
	graph->AddTextureResource( "prefilter", std::move( prefilter ) );
	graph->AddTextureResource( "brdf", std::move( brdf_integration ) );

	// add passes
	graph->AddPass( "crt", std::make_unique<ClearRenderTargetPass>() );
	graph->BindToResource( "crt.buffer", "target" );
	graph->BindToResource( "crt.depth", "depth" );

	graph->AddPass( "shadows", std::make_unique<ShadowPass>() );

	if( opaque_pass )
	{
		graph->AddPass( "opaque", std::make_unique<OpaquePass>() );
		graph->BindToResource( "opaque.dst", "target" );
		graph->BindToResource( "opaque.irradiance", "irradiance" );
		graph->BindToResource( "opaque.prefilter", "prefilter" );
		graph->BindToResource( "opaque.brdf", "brdf" );
	}

	if( transparent_pass )
	{
		graph->AddPass( "transparent", std::make_unique<TransparentPass>() );
		graph->BindToResource( "transparent.dst", "target" );
		graph->BindToResource( "transparent.irradiance", "irradiance" );
		graph->BindToResource( "transparent.prefilter", "prefilter" );
		graph->BindToResource( "transparent.brdf", "brdf" );
	}

	graph->AddPass( "draw_lights", std::make_unique<DrawLightsPass>() );

	if( skybox_enabled )
	{
		graph->BindToResource( "draw_lights.dst", "target" );

		graph->AddPass( "skybox", std::make_unique<SkyboxPass>() );
		graph->BindToResource( "skybox.skyboxtex", "skybox" );
	}
	if( !post_process_count )
	{
		if( msaa_enabled )
		{
			graph->AddPass( "resolve", std::make_unique<MsaaResolvePass>() );
			graph->BindToResource( "resolve.src", "target" );
			graph->BindToResource( "resolve.dst", "resolve_target" );
		}
		else if( skybox_enabled )
		{
			graph->MarkOutput( "skybox.dst" );
		}
		else
		{
			graph->MarkOutput( "draw_lights.dst" );
		}
	}
	else
	{
		if( skybox_enabled )
		{
			graph->BindToResource( "skybox.dst", "target" );
		}
		else
		{
			graph->BindToResource( "draw_lights.dst", "target" );
		}
		std::string input_rt = "target";

		if( msaa_enabled )
		{
			graph->AddPass( "resolve", std::make_unique<MsaaResolvePass>() );
			graph->BindToResource( "resolve.src", input_rt );
			graph->BindToResource( "resolve.dst", "resolve_target" );
			input_rt = "resolve_target";
		}

		if( bloom_enabled )
		{
			post_process_count--;

			auto bloom = std::make_unique<BloomPass>();
			bloom->SetThreshold( bloom_threshold );

			graph->AddPass( "bloom", std::move( bloom ) );
			graph->BindToResource( "bloom.src", input_rt );
			if( !post_process_count )
			{
				graph->MarkOutput( "bloom.dst" );
			}
			else
			{
				graph->BindToResource( "bloom.dst", "target2" );
			}

			input_rt = "target2";
		}

		if( motion_blur_enabled )
		{
			post_process_count--;

			graph->AddPass( "motionblur", std::make_unique<MotionBlurPass>() );
			graph->BindToResource( "motionblur.src", input_rt );
			graph->BindToResource( "motionblur.depth", "depth" );
			if( !post_process_count )
			{
				graph->MarkOutput( "motionblur.dst" );
			}
			else
			{
				graph->BindToResource( "motionblur.dst", "target" );
			}

			input_rt = "target";
		}

		if( tonemapping_enabled )
		{
			post_process_count--;

			auto tm = std::make_unique<ToneMappingPass>();
			tm->SetExposure( exposure );

			graph->AddPass( "tonemapping", std::move( tm ) );
			graph->BindToResource( "tonemapping.src", input_rt );

			graph->MarkOutput( "tonemapping.dst" );
		}
	}
}

void RenderBuilder::DrawSettings( const Bat::Renderer& gfx, const Bat::Window& wnd, Bat::RenderGraph* graph )
{
	bool changed = false;

	changed |= ImGui::Checkbox( "Opaque pass", &opaque_pass );
	changed |= ImGui::Checkbox( "Transparent pass", &transparent_pass );

	changed |= ImGui::Checkbox( "Skybox", &skybox_enabled );
	if( skybox_enabled && graph->GetPassByName( "skybox" ) )
	{
		auto skybox = static_cast<SkyboxPass*>( graph->GetPassByName( "skybox" ) );
		bool dynamic_sky = skybox->IsDynamicSkyEnabled();

		if( ImGui::Checkbox( "Dynamic sky", &dynamic_sky ) )
		{
			skybox->SetDynamicSkyEnabled( dynamic_sky );
		}

		if( !dynamic_sky )
		{
			if( ImGui::Button( "Load texture" ) )
			{
				auto path = FileDialog::Open( "Assets" );
				if( path )
				{
					skybox_tex = path->string();
					changed = true;
				}
			}
		}
	}

	changed |= ImGui::Checkbox( "Bloom", &bloom_enabled );
	if( bloom_enabled )
	{
		if( ImGui::SliderFloat( "Bloom threshold", &bloom_threshold, 0.0f, 100.0f ) )
		{
			auto bloom = static_cast<BloomPass*>( graph->GetPassByName( "bloom" ) );
			bloom->SetThreshold( bloom_threshold );
		}
	}

	changed |= ImGui::Checkbox( "Motion blur", &motion_blur_enabled );
	changed |= ImGui::Checkbox( "Tonemapping", &tonemapping_enabled );
	if( tonemapping_enabled )
	{
		if( ImGui::SliderFloat( "Exposure", &exposure, 0.0f, 32.0f ) )
		{
			auto tm = static_cast<ToneMappingPass*>( graph->GetPassByName( "tonemapping" ) );
			tm->SetExposure( exposure );
		}
	}

	changed |= ImGui::Checkbox( "MSAA", &msaa_enabled );
	if( msaa_enabled )
	{
		static int current_msaa_sample = 2;
		static const char* samples[] = { "1", "2", "4", "8" };
		if( ImGui::Combo( "MSAA Samples", &current_msaa_sample, samples, ARRAYSIZE( samples ) ) )
		{
			changed = true;
			msaa_samples = std::stoi( samples[current_msaa_sample] );
		}
	}

	if( changed )
	{
		Make( gfx, wnd, graph );
	}
}
