#include "PCH.h"
#include "Renderer.h"

#include "D3DGPUDevice.h"

#include "GraphicsFormats.h"

#include "MathLib.h"
#include "Camera.h"
#include "Texture.h"
#include "Light.h"
#include "RenderGraph.h"
#include "Platform/Window.h"
#include "ShaderManager.h"
#include "Events/WindowEvents.h"
#include "DebugDraw.h"
#include "Core/Scene.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

namespace Bat
{
	IGPUDevice* gpu = nullptr;

	Renderer::Renderer( Window& wnd )
		:
		m_UI( wnd )
	{
		gpu = CreateD3DGPUDevice( wnd, VSyncEnabled, ScreenFar, ScreenNear );

		InitialiseResources( wnd.GetWidth(), wnd.GetHeight() );

		wnd.OnEventDispatched<WindowResizeEvent>( [=]( const WindowResizeEvent& e )
		{
			Resize( e.width, e.height );
		} );

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;
		//io.ConfigViewportsNoDefaultParent = true;
		//io.ConfigDockingAlwaysTabBar = true;
		//io.ConfigDockingTransparentPayload = true;

		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplWin32_Init( wnd.GetHandle() );
		ImGui_ImplDX11_Init( (ID3D11Device*)gpu->GetImpl(), (ID3D11DeviceContext*)gpu->GetContext()->GetImpl() );

		BAT_TRACE( "ImGui initialized" );
	}

	Renderer::~Renderer()
	{
		delete gpu;

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		BAT_TRACE( "ImGui shut down" );
	}

	void Renderer::Resize( size_t width, size_t height )
	{
		InitialiseResources( width, height );

		gpu->ResizeBuffers( width, height );
	}

	void Renderer::SetRenderGraph( RenderGraph* graph )
	{
		m_pRenderGraph = graph;
	}

	void Renderer::BeginFrame()
	{
		if( m_pCamera )
		{
			m_pCamera->Render();
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ShaderManager::BindShaderGlobals( m_pCamera, { (float)m_iScreenWidth, (float)m_iScreenHeight }, gpu->GetContext() );
	}

	void Renderer::EndFrame()
	{
		RenderScene();
		if( m_pCamera )
		{
			DebugDraw::Flush( *m_pCamera );
		}
		RenderUI();
		RenderImGui();

		// all done!
		gpu->SwapBuffers();
	}

	Mat4 Renderer::GetOrthoMatrix() const
	{
		return m_matOrtho;
	}

	void Renderer::InitialiseResources( size_t width, size_t height )
	{
		m_iScreenWidth = (int)width;
		m_iScreenHeight = (int)height;
		m_matOrtho = Mat4::Ortho(
			(float)width,
			(float)height,
			Renderer::ScreenNear,
			Renderer::ScreenFar
		);

		Viewport vp;
		vp.width = (float)width;
		vp.height = (float)height;
		vp.min_depth = 0.0f;
		vp.max_depth = 1.0f;
		vp.top_left = { 0.0f, 0.0f };
		gpu->GetContext()->SetViewport( vp );
	}

	void Renderer::RenderScene()
	{
		if( m_pSceneGraph && m_pCamera && m_pRenderGraph )
		{
			m_pRenderGraph->Render( *m_pCamera, *m_pSceneGraph, gpu->GetBackbuffer() );
		}
	}

	void Renderer::RenderUI()
	{
		IGPUContext* pContext = gpu->GetContext();

		pContext->BeginEvent( "ultralight ui" );
		m_UI.Update();
		m_UI.Render();
		pContext->EndEvent();
	}

	void Renderer::RenderImGui()
	{
		IGPUContext* pContext = gpu->GetContext();

		pContext->BeginEvent( "imgui" );

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );

		ImGuiIO& io = ImGui::GetIO();
		if( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		pContext->EndEvent();
	}
}