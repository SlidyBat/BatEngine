#include "PCH.h"
#include "Graphics.h"

#include "D3DGPUDevice.h"

#include "GraphicsFormats.h"
#include "TexturePipeline.h"
#include "LightPipeline.h"
#include "ColourPipeline.h"
#include "BumpMapPipeline.h"
#include "SkyboxPipeline.h"

#include "MathLib.h"
#include "Camera.h"
#include "Texture.h"
#include "Light.h"
#include "Scene.h"
#include "RenderGraph.h"
#include "Window.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "ShaderManager.h"
#include "WindowEvents.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

namespace Bat
{
	IGPUDevice* gpu = nullptr;

	void Graphics::AddSamplers()
	{
		SamplerDesc sampler_desc;
		sampler_desc.filter = SampleFilter::MIN_MAG_MIP_LINEAR;
		sampler_desc.mip_lod_bias = 0.0f;
		sampler_desc.max_anisotropy = 1;
		sampler_desc.comparison_func = ComparisonFunc::ALWAYS;
		sampler_desc.border_color[0] = 0.0f;
		sampler_desc.border_color[1] = 0.0f;
		sampler_desc.border_color[2] = 0.0f;
		sampler_desc.border_color[3] = 0.0f;

		// wrap sampler
		sampler_desc.address_u = TextureAddressMode::WRAP;
		sampler_desc.address_v = TextureAddressMode::WRAP;
		sampler_desc.address_w = TextureAddressMode::WRAP;
		m_pSamplers.emplace_back( gpu->CreateSampler( sampler_desc ) );

		// clamp sampler
		sampler_desc.address_u = TextureAddressMode::CLAMP;
		sampler_desc.address_v = TextureAddressMode::CLAMP;
		sampler_desc.address_w = TextureAddressMode::CLAMP;
		m_pSamplers.emplace_back( gpu->CreateSampler( sampler_desc ) );	
	}

	void Graphics::BindSamplers()
	{
		for( size_t i = 0; i < m_pSamplers.size(); i++ )
		{
			gpu->GetContext()->SetSampler( ShaderType::PIXEL, m_pSamplers[i].get(), i );
		}
	}

	Graphics::Graphics( Window& wnd )
		:
		m_UI( wnd )
	{
		gpu = CreateD3DGPUDevice( wnd, VSyncEnabled, ScreenFar, ScreenNear );

		Viewport vp;
		vp.width = (float)wnd.GetWidth();
		vp.height = (float)wnd.GetHeight();
		vp.min_depth = 0.0f;
		vp.max_depth = 1.0f;
		vp.top_left = { 0.0f, 0.0f };
		gpu->GetContext()->SetViewport( vp );

		ShaderManager::AddPipeline( "texture", std::make_unique<TexturePipeline>( "Graphics/Shaders/TextureVS.hlsl", "Graphics/Shaders/TexturePS.hlsl" ) );
		ShaderManager::AddPipeline( "colour", std::make_unique <ColourPipeline>( "Graphics/Shaders/ColourVS.hlsl", "Graphics/Shaders/ColourPS.hlsl" ) );
		ShaderManager::AddPipeline( "light", std::make_unique<LightPipeline>( "Graphics/Shaders/LightVS.hlsl", "Graphics/Shaders/LightPS.hlsl" ) );
		ShaderManager::AddPipeline( "bumpmap", std::make_unique<BumpMapPipeline>( "Graphics/Shaders/BumpMapVS.hlsl", "Graphics/Shaders/BumpMapPS.hlsl" ) );
		ShaderManager::AddPipeline( "skybox", std::make_unique<SkyboxPipeline>( "Graphics/Shaders/SkyboxVS.hlsl", "Graphics/Shaders/SkyboxPS.hlsl" ) );

		AddSamplers();

		wnd.OnEventDispatched<WindowResizeEvent>( [=]( const WindowResizeEvent& e )
		{
			Resize( e.width, e.height );
		} );

		m_matOrtho = DirectX::XMMatrixOrthographicLH(
			(float)wnd.GetWidth(),
			(float)wnd.GetHeight(),
			Graphics::ScreenNear,
			Graphics::ScreenFar
		);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui_ImplWin32_Init( wnd.GetHandle() );
		ImGui_ImplDX11_Init( (ID3D11Device*)gpu->GetImpl(), (ID3D11DeviceContext*)gpu->GetContext()->GetImpl() );
		ImGui::StyleColorsDark();
		BAT_TRACE( "ImGui initialized" );
	}

	Graphics::~Graphics()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		BAT_TRACE( "ImGui shut down" );
	}

	void Graphics::Resize( int width, int height )
	{
		m_iScreenWidth = width;
		m_iScreenHeight = height;
		m_matOrtho = DirectX::XMMatrixOrthographicLH(
			(float)width,
			(float)height,
			Graphics::ScreenNear,
			Graphics::ScreenFar
		);
	}

	void Graphics::SetRenderGraph( RenderGraph* graph )
	{
		m_pRenderGraph = graph;
	}

	void Graphics::BeginFrame()
	{
		if( m_pSceneGraph && m_pSceneGraph->GetActiveCamera() )
		{
			m_pSceneGraph->GetActiveCamera()->Render();
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		BindSamplers();
	}

	void Graphics::EndFrame()
	{
		RenderScene();
		RenderUI();
		RenderImGui();

		// all done!
		gpu->SwapBuffers();
	}

	DirectX::XMMATRIX Graphics::GetOrthoMatrix() const
	{
		return m_matOrtho;
	}

	void Graphics::RenderScene()
	{
		if( m_pSceneGraph && m_pRenderGraph )
		{
			// nullptr RT is backbuffer
			m_pRenderGraph->Render( *m_pSceneGraph, nullptr );
		}
	}

	void Graphics::RenderUI()
	{
		m_UI.Update();
		m_UI.Render();
	}

	void Graphics::RenderImGui()
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
	}
}