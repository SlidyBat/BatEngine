#include "Graphics.h"
#include "VertexTypes.h"
#include "TexturePipeline.h"
#include "LightPipeline.h"
#include "ColourPipeline.h"
#include "IModel.h"
#include "Window.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

namespace Bat
{
	IGraphics* g_pGfx = nullptr;

	Graphics::Graphics( Window& wnd )
		:
		d3d( wnd, VSyncEnabled, ScreenFar, ScreenNear )
	{
		IGraphics::RegisterGraphics( this );
		AddShader( "texture", new TexturePipeline( L"Graphics/Shaders/TextureVS.hlsl", L"Graphics/Shaders/TexturePS.hlsl" ) );
		AddShader( "colour", new ColourPipeline( L"Graphics/Shaders/ColourVS.hlsl", L"Graphics/Shaders/ColourPS.hlsl" ) );
		AddShader( "light", new LightPipeline( L"Graphics/Shaders/LightVS.hlsl", L"Graphics/Shaders/LightPS.hlsl" ) );

		wnd.AddResizeListener( [=]( int width, int height )
		{
			Resize( width, height );
			m_pCamera->SetAspectRatio( (float)width / height );
		} );

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui_ImplWin32_Init( wnd.GetHandle() );
		ImGui_ImplDX11_Init( GetDevice(), GetDeviceContext() );
		ImGui::StyleColorsDark();
	}

	Graphics::~Graphics()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	IModel* Graphics::CreateColouredModel( const std::vector<ColourVertex>& vertices, const std::vector<int>& indices )
	{
		ColourMesh mesh( vertices, indices, nullptr );
		return new ColouredModel( mesh );
	}

	IModel* Graphics::CreateTexturedModel( const std::vector<TexVertex>& vertices, const std::vector<int>& indices, Texture& tex )
	{
		TexMesh mesh( vertices, indices, &tex );
		return new TexturedModel( mesh );
	}

	IModel* Graphics::CreateModel( const std::vector<Vertex>& vertices, const std::vector<int>& indices, Texture& tex )
	{
		Mesh mesh( vertices, indices, &tex );
		return new LightModel( mesh );
	}

	Texture* Graphics::CreateTexture( const std::wstring& filename )
	{
		return new Texture( filename );
	}

	Texture* Graphics::CreateTexture( const Colour* pPixels, int width, int height )
	{
		return new Texture( pPixels, width, height );
	}

	bool Graphics::IsDepthStencilEnabled() const
	{
		return d3d.IsDepthStencilEnabled();
	}

	void Graphics::SetDepthStencilEnabled( bool enable )
	{
		d3d.SetDepthStencilEnabled( enable );
	}

	void Graphics::BeginFrame()
	{
		d3d.BeginScene( 0.1f, 0.1f, 0.1f, 1.0f );

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void Graphics::EndFrame()
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );

		m_pCamera->Render();
		d3d.EndScene();
	}
}