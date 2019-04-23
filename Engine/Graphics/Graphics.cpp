#include "PCH.h"
#include "Graphics.h"

#include "D3DGPUDevice.h"

#include "GraphicsFormats.h"
#include "TexturePipeline.h"
#include "ColourPipeline.h"
#include "LitGenericPipeline.h"
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
		ShaderManager::AddPipeline( "litgeneric", std::make_unique<LitGenericPipeline>( "Graphics/Shaders/LitGenericVS.hlsl", "Graphics/Shaders/LitGenericPS.hlsl" ) );
		ShaderManager::AddPipeline( "skybox", std::make_unique<SkyboxPipeline>( "Graphics/Shaders/SkyboxVS.hlsl", "Graphics/Shaders/SkyboxPS.hlsl" ) );

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
		delete gpu;

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

		ShaderManager::BindShaderGlobals( m_pSceneGraph->GetActiveCamera(), { (float)m_iScreenWidth, (float)m_iScreenHeight }, gpu->GetContext() );
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