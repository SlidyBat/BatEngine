#include "PCH.h"
#include "Graphics.h"

#include <d3d11.h>

#include "VertexTypes.h"
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
#include "Model.h"
#include "Window.h"
#include "Material.h"
#include "IPipeline.h"
#include "IPostProcess.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "WindowEvents.h"
#include "BloomPostProcess.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include "RenderContext.h"

namespace Bat
{
	Graphics::Graphics( Window& wnd )
		:
		d3d( wnd, VSyncEnabled, ScreenFar, ScreenNear )
	{
		RenderContext::SetD3DClass( d3d );

		AddShader( "texture", std::make_unique<TexturePipeline>( "Graphics/Shaders/TextureVS.hlsl", "Graphics/Shaders/TexturePS.hlsl" ) );
		AddShader( "colour", std::make_unique <ColourPipeline>( "Graphics/Shaders/ColourVS.hlsl", "Graphics/Shaders/ColourPS.hlsl" ) );
		AddShader( "light", std::make_unique<LightPipeline>( "Graphics/Shaders/LightVS.hlsl", "Graphics/Shaders/LightPS.hlsl" ) );
		AddShader( "bumpmap", std::make_unique<BumpMapPipeline>( "Graphics/Shaders/BumpMapVS.hlsl", "Graphics/Shaders/BumpMapPS.hlsl" ) );
		AddShader( "skybox", std::make_unique<SkyboxPipeline>( "Graphics/Shaders/SkyboxVS.hlsl", "Graphics/Shaders/SkyboxPS.hlsl" ) );

		m_pBloomProcess = std::make_unique<BloomPostProcess>( wnd.GetWidth(), wnd.GetHeight() );

		m_FrameBuffers[0].Resize( wnd.GetWidth(), wnd.GetHeight() );
		m_FrameBuffers[1].Resize( wnd.GetWidth(), wnd.GetHeight() );

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

		m_FrameBuffers[0].Resize( width, height );
		m_FrameBuffers[1].Resize( width, height );
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

		// dont render directly to backbuffer if we have any post effects to do
		if( !m_PostProcesses.empty() || m_bBloomEnabled )
		{
			m_FrameBuffers[0].Bind(); // draw to texture
			m_FrameBuffers[0].Clear( 0.3f, 0.3f, 0.3f, 1.0f );
		}
		else
		{
			d3d.BindBackBuffer(); // draw directly to screen
			d3d.ClearScene( 0.3f, 0.3f, 0.3f, 1.0f );
		}
	}

	void Graphics::EndFrame()
	{
		// render scene
		RenderScene();

		// render skybox
		if( m_pSkybox )
		{
			auto pos = GetActiveCamera()->GetPosition();
			auto w = DirectX::XMMatrixTranslation( pos.x, pos.y, pos.z );
			auto t = w * GetActiveCamera()->GetViewMatrix() * GetActiveCamera()->GetProjectionMatrix();

			SkyboxPipelineParameters params( t, m_pSkybox->GetTextureView() );
			auto pPipeline = GetPipeline( "skybox" );
			pPipeline->BindParameters( params );
			pPipeline->RenderIndexed( 0 ); // skybox uses its own index buffer & index count, doesnt matter what we pass in
		}

		// bloom
		if( m_bBloomEnabled )
		{
			m_pBloomProcess->Render( m_FrameBuffers[0] );
		}

		// post processes
		if( !m_PostProcesses.empty() )
		{
			DisableDepthStencil();

			RenderTexture* pCurrentTexture = &m_FrameBuffers[0];
			if( m_PostProcesses.size() > 1 )
			{
				m_FrameBuffers[1].Clear( 0.0f, 0.0f, 0.0f, 1.0f );
				m_FrameBuffers[1].Bind();

				for( size_t i = 0; i < m_PostProcesses.size() - 1; i++ )
				{
					m_PostProcesses[i]->Render( *pCurrentTexture );

					if( i % 2 == 0 )
					{
						pCurrentTexture = &m_FrameBuffers[1];
						m_FrameBuffers[0].Clear( 0.0f, 0.0f, 0.0f, 1.0f );
						m_FrameBuffers[0].Bind();
					}
					else
					{
						pCurrentTexture = &m_FrameBuffers[0];
						m_FrameBuffers[1].Clear( 0.0f, 0.0f, 0.0f, 1.0f );
						m_FrameBuffers[1].Bind();
					}
				}
			}

			// render last post process directly to screen
			d3d.BindBackBuffer();
			m_PostProcesses.back()->Render( *pCurrentTexture );

			EnableDepthStencil();
		}

		// text commands
		if( !m_TextDrawCommands.empty() )
		{
			m_pSpriteBatch->Begin();
			for( const auto& command : m_TextDrawCommands )
			{
				m_pFont->DrawString( m_pSpriteBatch.get(), command.text.c_str(), command.pos, command.col );
			}
			m_pSpriteBatch->End();
		}

		// imgui
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );

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

	class RenderSceneVisitor : public ISceneVisitor
	{
	public:
		RenderSceneVisitor( Graphics& gfx, Light* pLight = nullptr )
			:
			gfx( gfx ),
			m_pLight( pLight )
		{}

		void SetLight( Light* pLight ) { m_pLight = pLight; }

		virtual void Visit( const DirectX::XMMATRIX& transform, ISceneNode& node )
		{
			size_t count = node.GetModelCount();
			for( size_t i = 0; i < count; i++ )
			{
				Model* pModel = node.GetModel( i );
				DirectX::XMMATRIX w = transform * pModel->GetWorldMatrix();
				DirectX::XMMATRIX vp = gfx.GetActiveCamera()->GetViewMatrix() * gfx.GetActiveCamera()->GetProjectionMatrix();

				pModel->Bind();

				auto& meshes = pModel->GetMeshes();
				for( auto& pMesh : meshes )
				{
					auto szPipelineName = pMesh->GetMaterial().GetDefaultPipelineName();
					auto pPipeline = static_cast<LightPipeline*>( gfx.GetPipeline( szPipelineName ) );
					pPipeline->SetLight( m_pLight );

					Material material = pMesh->GetMaterial();
					LightPipelineParameters params( *gfx.GetActiveCamera(), w, vp, material );
					pMesh->Bind( pPipeline );
					pPipeline->BindParameters( params );
					pPipeline->RenderIndexed( (UINT)pMesh->GetIndexCount() );
				}
			}
		}
	private:
		Graphics& gfx;
		Light* m_pLight = nullptr;
	};

	void Graphics::RenderScene()
	{
		static RenderSceneVisitor scene_renderer( *this );

		Light* pLight = nullptr;
		GetSceneLightsVisitor obtain_light( &pLight );
		m_pSceneGraph->AcceptVisitor( obtain_light );

		EnableDepthStencil();
		scene_renderer.SetLight( pLight );
		m_pSceneGraph->AcceptVisitor( scene_renderer );
	}
}