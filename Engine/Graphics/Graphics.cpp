#include "PCH.h"
#include "Graphics.h"

#include <d3d11.h>

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
#include "IPipeline.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "WindowEvents.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include "RenderContext.h"

namespace Bat
{
	static void AddSamplers()
	{
		D3D11_SAMPLER_DESC sampler_desc;
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampler_desc.MipLODBias = 0.0f;
		sampler_desc.MaxAnisotropy = 1;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampler_desc.BorderColor[0] = 0.0f;
		sampler_desc.BorderColor[1] = 0.0f;
		sampler_desc.BorderColor[2] = 0.0f;
		sampler_desc.BorderColor[3] = 0.0f;
		sampler_desc.MinLOD = 0;
		sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

		// wrap sampler
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		RenderContext::AddSampler( sampler_desc );

		// clamp sampler
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		RenderContext::AddSampler( sampler_desc );	
	}

	Graphics::Graphics( Window& wnd )
		:
		d3d( wnd, VSyncEnabled, ScreenFar, ScreenNear ),
		m_UI( wnd )
	{
		RenderContext::SetD3DClass( d3d );

		AddShader( "texture", std::make_unique<TexturePipeline>( "Graphics/Shaders/TextureVS.hlsl", "Graphics/Shaders/TexturePS.hlsl" ) );
		AddShader( "colour", std::make_unique <ColourPipeline>( "Graphics/Shaders/ColourVS.hlsl", "Graphics/Shaders/ColourPS.hlsl" ) );
		AddShader( "light", std::make_unique<LightPipeline>( "Graphics/Shaders/LightVS.hlsl", "Graphics/Shaders/LightPS.hlsl" ) );
		AddShader( "bumpmap", std::make_unique<BumpMapPipeline>( "Graphics/Shaders/BumpMapVS.hlsl", "Graphics/Shaders/BumpMapPS.hlsl" ) );
		AddShader( "skybox", std::make_unique<SkyboxPipeline>( "Graphics/Shaders/SkyboxVS.hlsl", "Graphics/Shaders/SkyboxPS.hlsl" ) );

		AddSamplers();

		m_pSpriteBatch = std::make_unique<DirectX::SpriteBatch>( GetDeviceContext() );
		m_pFont = std::make_unique<DirectX::SpriteFont>( GetDevice(), L"Assets/Fonts/consolas.spritefont" );

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
		ImGui_ImplDX11_Init( GetDevice(), GetDeviceContext() );
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

		d3d.Resize( width, height );
	}

	int Graphics::GetScreenWidth() const
	{
		return m_iScreenWidth;
	}

	int Graphics::GetScreenHeight() const
	{
		return m_iScreenHeight;
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

	void Graphics::SetRenderGraph( RenderGraph* graph )
	{
		m_pRenderGraph = graph;
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
		if( m_pCamera )
		{
			m_pCamera->Render();
		}

		m_TextDrawCommands.clear();
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void Graphics::EndFrame()
	{
		RenderScene();
		RenderText();
		RenderUI();
		RenderImGui();

		// all done!
		d3d.PresentScene();
	}

	void Graphics::DrawText( std::wstring text, const Vec2& pos, const DirectX::FXMVECTOR col/* = DirectX::Colors::White */ )
	{
		m_TextDrawCommands.emplace_back( std::move( text ), pos, col );
	}

	DirectX::XMMATRIX Graphics::GetOrthoMatrix() const
	{
		return m_matOrtho;
	}

	ID3D11Device * Graphics::GetDevice() const
	{
		return d3d.GetDevice();
	}

	ID3D11DeviceContext * Graphics::GetDeviceContext() const
	{
		return d3d.GetDeviceContext();
	}

	void Graphics::AddShader( const std::string & name, std::unique_ptr<IPipeline> pPipeline )
	{
		m_mapPipelines[name] = std::move( pPipeline );
	}

	

	class GetSceneLightsVisitor : public ISceneVisitor
	{
	public:
		GetSceneLightsVisitor( Light** ppLight )
			:
			m_ppLight( ppLight )
		{}

		virtual void Visit( const DirectX::XMMATRIX& transform, ISceneNode& node )
		{
			size_t count = node.GetLightCount();
			for( size_t i = 0; i < count; i++ )
			{
				*m_ppLight = node.GetLight( i );
			}
		}
	private:
		Light** m_ppLight;
	};

	void Graphics::RenderScene()
	{
		RenderTexture backbuffer = RenderTexture::Backbuffer();

		if( m_pSceneGraph && m_pRenderGraph )
		{
			m_pRenderGraph->Render( *m_pSceneGraph, backbuffer );
		}
	}

	//void Graphics::RenderSkybox()
	//{
	//	if( m_pSkybox )
	//	{
	//		auto pos = GetActiveCamera()->GetPosition();
	//		auto w = DirectX::XMMatrixTranslation( pos.x, pos.y, pos.z );
	//		auto t = w * GetActiveCamera()->GetViewMatrix() * GetActiveCamera()->GetProjectionMatrix();

	//		SkyboxPipelineParameters params( t, m_pSkybox->GetTextureView() );
	//		auto pPipeline = GetPipeline( "skybox" );
	//		pPipeline->BindParameters( params );
	//		pPipeline->RenderIndexed( 0 ); // skybox uses its own index buffer & index count, doesnt matter what we pass in
	//	}
	//}

	void Graphics::RenderText()
	{
		if( !m_TextDrawCommands.empty() )
		{
			m_pSpriteBatch->Begin();
			for( const auto& command : m_TextDrawCommands )
			{
				m_pFont->DrawString( m_pSpriteBatch.get(), command.text.c_str(), command.pos, command.col );
			}
			m_pSpriteBatch->End();
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