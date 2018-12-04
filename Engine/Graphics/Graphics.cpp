#include "Graphics.h"
#include "VertexTypes.h"
#include "TexturePipeline.h"
#include "LightPipeline.h"
#include "ColourPipeline.h"
#include "BumpMapPipeline.h"
#include "SkyboxPipeline.h"
#include "IModel.h"
#include "Window.h"
#include "Material.h"
#include "IPipeline.h"
#include "IPostProcess.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "WindowEvents.h"

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
		AddShader( "skybox", std::make_unique<SkyboxPipeline>( L"Graphics/Shaders/Build/SkyboxVS.cso", L"Graphics/Shaders/Build/SkyboxPS.cso" ) );

		m_pSpriteBatch = std::make_unique<DirectX::SpriteBatch>( GetDeviceContext() );
		m_pFont = std::make_unique<DirectX::SpriteFont>( GetDevice(), L"Assets/Fonts/consolas.spritefont" );

		ON_EVENT_DISPATCHED( [=]( const WindowResizeEvent* e )
		{
			Resize( e->GetWidth(), e->GetHeight() );
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
	}

	Graphics::~Graphics()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
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
		const size_t size = m_PostProcesses.size();
		if( size == 0 )
		{
			m_PostProcessRenderTexture.Resize( width, height );
		}
		else if( size == 1 )
		{
			m_AlternatePostProcessRenderTexture.Resize( width, height );
		}
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

	void Graphics::AddPostProcess( std::unique_ptr<IPostProcess> pPostProcess )
	{
		const size_t size = m_PostProcesses.size();
		if( size == 0 )
		{
			// only allocate render texture once we have a postprocess
			m_PostProcessRenderTexture.Resize( m_iScreenWidth, m_iScreenHeight );
		}
		else if( size == 1 )
		{
			// allocate second render texture for the multiple postprocesses to ping-pong between
			m_AlternatePostProcessRenderTexture.Resize( m_iScreenWidth, m_iScreenHeight );
		}

		m_PostProcesses.emplace_back( std::move( pPostProcess ) );
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
		m_pCamera->Render();
		m_TextDrawCommands.clear();
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if( !m_PostProcesses.empty() )
		{
			m_PostProcessRenderTexture.Bind(); // draw to texture
			m_PostProcessRenderTexture.Clear( 0.3f, 0.3f, 0.3f, 1.0f );
		}
		else
		{
			d3d.BindBackBuffer(); // draw directly to screen
			d3d.ClearScene( 0.3f, 0.3f, 0.3f, 1.0f );
		}
	}

	void Graphics::EndFrame()
	{
		if( m_pSkybox )
		{
			auto pos = GetCamera()->GetPosition();
			auto w = DirectX::XMMatrixTranslation( pos.x, pos.y, pos.z );
			auto t = w * GetCamera()->GetViewMatrix() * GetCamera()->GetProjectionMatrix();

			SkyboxPipelineParameters params( t, m_pSkybox->GetTextureView() );
			auto pPipeline = GetPipeline( "skybox" );
			pPipeline->BindParameters( &params );
			pPipeline->RenderIndexed( 0 ); // skybox uses its own index buffer & index count, doesnt matter what we pass in
		}

		if( !m_PostProcesses.empty() )
		{
			DisableDepthStencil();

			ID3D11ShaderResourceView* pCurrentTexture = m_PostProcessRenderTexture.GetTextureView();
			if( m_PostProcesses.size() > 1 )
			{
				m_AlternatePostProcessRenderTexture.Clear( 0.0f, 0.0f, 0.0f, 1.0f );
				m_AlternatePostProcessRenderTexture.Bind();

				for( size_t i = 0; i < m_PostProcesses.size() - 1; i++ )
				{
					m_PostProcesses[i]->Render( pCurrentTexture );

					if( i % 2 == 0 )
					{
						pCurrentTexture = m_AlternatePostProcessRenderTexture.GetTextureView();
						m_PostProcessRenderTexture.Clear( 0.0f, 0.0f, 0.0f, 1.0f );
						m_PostProcessRenderTexture.Bind();
					}
					else
					{
						pCurrentTexture = m_PostProcessRenderTexture.GetTextureView();
						m_AlternatePostProcessRenderTexture.Clear( 0.0f, 0.0f, 0.0f, 1.0f );
						m_AlternatePostProcessRenderTexture.Bind();
					}
				}
			}

			// render last post process directly to screen
			d3d.BindBackBuffer();
			m_PostProcesses.back()->Render( pCurrentTexture );

			EnableDepthStencil();
		}

		if( !m_TextDrawCommands.empty() )
		{
			m_pSpriteBatch->Begin();
			for( const auto& command : m_TextDrawCommands )
			{
				m_pFont->DrawString( m_pSpriteBatch.get(), command.text.c_str(), command.pos, command.col );
			}
			m_pSpriteBatch->End();
		}

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );

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

	ID3D11RenderTargetView * Graphics::GetRenderTargetView() const
	{
		return d3d.GetRenderTargetView();
	}

	ID3D11DepthStencilView * Graphics::GetDepthStencilView() const
	{
		return d3d.GetDepthStencilView();
	}

	void Graphics::AddShader( const std::string & name, std::unique_ptr<IPipeline> pPipeline )
	{
		m_mapPipelines[name] = std::move( pPipeline );
	}
}