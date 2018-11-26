#include "Graphics.h"
#include "VertexTypes.h"
#include "TexturePipeline.h"
#include "LightPipeline.h"
#include "ColourPipeline.h"
#include "BumpMapPipeline.h"
#include "IModel.h"
#include "Window.h"
#include "Material.h"
#include "IPipeline.h"

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
		AddShader( "texture", std::make_unique<TexturePipeline>( L"Graphics/Shaders/Build/TextureVS.cso", L"Graphics/Shaders/Build/TexturePS.cso" ) );
		AddShader( "colour", std::make_unique <ColourPipeline>( L"Graphics/Shaders/Build/ColourVS.cso", L"Graphics/Shaders/Build/ColourPS.cso" ) );
		AddShader( "light", std::make_unique<LightPipeline>( L"Graphics/Shaders/Build/LightVS.cso", L"Graphics/Shaders/Build/LightPS.cso" ) );
		AddShader( "bumpmap", std::make_unique<BumpMapPipeline>( L"Graphics/Shaders/Build/BumpMapVS.cso", L"Graphics/Shaders/Build/BumpMapPS.cso" ) );

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

	IPipeline* Graphics::GetPipeline( const std::string & name ) const
	{
		auto it = m_mapPipelines.find( name );
		if( it == m_mapPipelines.end() )
		{
			return nullptr;
		}

		return it->second.get();
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
		d3d.BeginScene( 0.3f, 0.3f, 0.3f, 1.0f );

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

	void Graphics::AddShader( const std::string & name, std::unique_ptr<IPipeline> pPipeline )
	{
		m_mapPipelines[name] = std::move( pPipeline );
	}
}