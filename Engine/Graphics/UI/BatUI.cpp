#include "PCH.h"
#include "BatUI.h"

#include "UI/UltralightAdapters/GPUDriverD3D11.h"
#include "UI/UltralightAdapters/PlatformD3D11.h"
#include "UI/UltralightAdapters/FontLoaderWin.h"
#include "UI/UltralightAdapters/FileSystemBasic.h"

#include "RenderContext.h"
#include "Common.h"
#include "Window.h"
#include "WindowEvents.h"

using namespace ultralight;
using namespace framework;

namespace Bat
{
	static ultralight::IndexType patternCW[] = { 0, 1, 3, 1, 2, 3 };
	static ultralight::IndexType patternCCW[] = { 0, 3, 1, 1, 3, 2 };

	class BatPlatformD3D11 : public PlatformGPUContextD3D11
	{
	public:
		BatPlatformD3D11( const Window& wnd )
			:
			m_Wnd( wnd )
		{}

		virtual ID3D11Device* device() override { return RenderContext::GetDevice(); }
		virtual ID3D11DeviceContext* immediate_context() override { return RenderContext::GetDeviceContext(); }
		virtual IDXGISwapChain* swap_chain() override { return RenderContext::GetSwapChain(); }
		virtual ID3D11RenderTargetView* render_target_view() override { return RenderContext::GetBackBufferView(); }

		// Scale is calculated from monitor DPI, see Application::SetScale
		virtual void set_scale( double scale ) override { m_flScale = scale; }
		virtual double scale() const override { return m_flScale; }

		// This is in virtual units, not actual pixels.
		virtual void set_screen_size(uint32_t width, uint32_t height) override
		{
			ASSERT( false, "Attempted to set screen size" );
		}
		virtual void screen_size(uint32_t& width, uint32_t& height) override
		{
			width  = (uint32_t)m_Wnd.GetWidth();
			height = (uint32_t)m_Wnd.GetHeight();
		}
	private:
		double m_flScale = 1.0f;

		const Window& m_Wnd;
	};

	Overlay::Overlay( ultralight::Ref<ultralight::Renderer> renderer, ultralight::GPUDriver* driver,
		Window& wnd, int width, int height, const Vei2& pos, float scale )
		:
		m_pView( renderer->CreateView( width, height, true ) ),
		m_iWidth( width ),
		m_iHeight( height ),
		m_vecPos( pos ),
		m_pDriver( driver )
	{
		wnd.AddEventListener<WindowResizeEvent>( *this );

		memset(&m_GPUState, 0, sizeof(m_GPUState));
		m_GPUState.viewport_width  = (float)wnd.GetWidth();
		m_GPUState.viewport_height = (float)wnd.GetHeight();
	}

	Overlay::~Overlay()
	{
		if( !m_Vertices.empty() )
		{
			m_pDriver->DestroyGeometry( m_iGeometryId );
		}
	}

	void Overlay::LoadHTMLRaw( const std::string& html )
	{
		m_pView->LoadHTML( html.c_str() );
	}

	void Overlay::LoadHTMLFromFile( const std::string& filename )
	{
		m_pView->LoadURL( ( "file:///" + filename ).c_str() );
		StartWatchingFile( filename );
	}

	void Overlay::LoadHTMLFromURL( const std::string& url )
	{
		m_pView->LoadURL( url.c_str() );
	}

	void Overlay::Draw()
	{
		UpdateGeometry();
		m_GPUState.texture_1_id = m_pView->render_target().texture_id;
		m_pDriver->DrawGeometry( m_iGeometryId, 6, 0, m_GPUState );
	}

	void Overlay::Resize( int width, int height )
	{
		if( width == m_iWidth && height == m_iHeight )
		{
			return;
		}

		m_pView->Resize( width, height );

		m_iWidth = width;
		m_iHeight = height;
		m_bDirty = true;
	}

	void Overlay::OnEvent( const WindowResizeEvent& e )
	{
		m_GPUState.viewport_width  = (float)e.width;
		m_GPUState.viewport_height = (float)e.height;
	}

	void Overlay::UpdateGeometry()
	{
		bool initial_creation = false;
		if (m_Vertices.empty())
		{
			m_Vertices.resize(4);
			m_Indices.resize(6);

			auto& config = ultralight::Platform::instance().config();
			if (config.face_winding == ultralight::kFaceWinding_Clockwise)
				memcpy(m_Indices.data(), patternCW, sizeof(ultralight::IndexType) * m_Indices.size());
			else
				memcpy(m_Indices.data(), patternCCW, sizeof(ultralight::IndexType) * m_Indices.size());

			ultralight::Matrix identity;
			identity.SetIdentity();

			m_GPUState.transform = ultralight::ConvertAffineTo4x4(identity);
			m_GPUState.enable_blend = true;
			m_GPUState.enable_texturing = true;
			m_GPUState.shader_type = ultralight::kShaderType_Fill;
			m_GPUState.render_buffer_id = 0; // default render target view (screen)
			m_GPUState.texture_1_id = m_pView->render_target().texture_id;

			initial_creation = true;
		}

		if (!m_bDirty)
			return;

		ultralight::Vertex_2f_4ub_2f_2f_28f v;
		memset(&v, 0, sizeof(v));

		v.data0[0] = 1; // Fill Type: Image

		v.color[0] = 255;
		v.color[1] = 255;
		v.color[2] = 255;
		v.color[3] = 255;

		float left = static_cast<float>(m_vecPos.x);
		float top = static_cast<float>(m_vecPos.y);
		float right = static_cast<float>(m_vecPos.x + Width());
		float bottom = static_cast<float>(m_vecPos.y + Height());

		ultralight::RenderTarget target = m_pView->render_target();
		float tex_width_ratio = target.width / (float)target.texture_width;
		float tex_height_ratio = target.height / (float)target.texture_height;

#ifdef FRAMEWORK_PLATFORM_GLFW
		const float flip_y = false;
#else
		const float flip_y = false;
#endif

		// TOP LEFT
		v.pos[0] = v.obj[0] = left;
		v.pos[1] = v.obj[1] = top;
		v.tex[0] = 0;
		v.tex[1] = flip_y ? tex_height_ratio : 0;

		m_Vertices[0] = v;

		// TOP RIGHT
		v.pos[0] = v.obj[0] = right;
		v.pos[1] = v.obj[1] = top;
		v.tex[0] = tex_width_ratio;
		v.tex[1] = flip_y ? tex_height_ratio : 0;

		m_Vertices[1] = v;

		// BOTTOM RIGHT
		v.pos[0] = v.obj[0] = right;
		v.pos[1] = v.obj[1] = bottom;
		v.tex[0] = tex_width_ratio;
		v.tex[1] = flip_y ? 0 : tex_height_ratio;

		m_Vertices[2] = v;

		// BOTTOM LEFT
		v.pos[0] = v.obj[0] = left;
		v.pos[1] = v.obj[1] = bottom;
		v.tex[0] = 0;
		v.tex[1] = flip_y ? 0 : tex_height_ratio;

		m_Vertices[3] = v;

		ultralight::VertexBuffer vbuffer;
		vbuffer.format = ultralight::kVertexBufferFormat_2f_4ub_2f_2f_28f;
		vbuffer.size = static_cast<uint32_t>(sizeof(ultralight::Vertex_2f_4ub_2f_2f_28f) * m_Vertices.size());
		vbuffer.data = (uint8_t*)m_Vertices.data();

		ultralight::IndexBuffer ibuffer;
		ibuffer.size = static_cast<uint32_t>(sizeof(ultralight::IndexType) * m_Indices.size());
		ibuffer.data = (uint8_t*)m_Indices.data();

		if (initial_creation)
		{
			m_iGeometryId = m_pDriver->NextGeometryId();
			m_pDriver->CreateGeometry(m_iGeometryId, vbuffer, ibuffer);
		}
		else
		{
			m_pDriver->UpdateGeometry(m_iGeometryId, vbuffer, ibuffer);
		}

		m_bDirty = false;
	}

	void Overlay::OnFileChanged( const std::string & filename )
	{
		m_pView->LoadURL( ( "file:///" + filename ).c_str() );
	}

	void Overlay::StartWatchingFile( const std::string & filename )
	{
		StopWatchingFile();
		m_iListenHandle = FileWatchdog::AddFileChangeListener( filename, BIND_MEM_FN( Overlay::OnFileChanged ) );
	}

	void Overlay::StopWatchingFile()
	{
		if( m_iListenHandle != FileWatchdog::INVALID_LISTENER )
		{
			FileWatchdog::RemoveFileChangeListener( m_iListenHandle );
			m_iListenHandle = FileWatchdog::INVALID_LISTENER;
		}
	}

	BatUI::BatUI( Window& wnd )
		:
		wnd( wnd ),
		gpu_context( std::make_unique<BatPlatformD3D11>( wnd ) )
	{
		config.face_winding = kFaceWinding_Clockwise;
		config.device_scale_hint = 1.0f;

		gpu_driver = std::make_unique<GPUDriverD3D11>( gpu_context.get() );

		font_loader = std::make_unique<FontLoaderWin>();

		filesystem = std::make_unique<FileSystemBasic>( "" );

		auto& platform = Platform::instance();
		platform.set_config( config );
		platform.set_gpu_driver( gpu_driver.get() );
		platform.set_font_loader( font_loader.get() );
		platform.set_file_system( filesystem.get() );

		renderer = Renderer::Create();
	}

	void BatUI::Update()
	{
		renderer->Update();
	}

	void BatUI::Render()
	{
		gpu_driver->BeginSynchronize();

		renderer->Render();

		gpu_driver->EndSynchronize();

		if( gpu_driver->HasCommandsPending() )
		{
			gpu_driver->DrawCommandList();
		}

		DrawOverlays();
	}

	Overlay* BatUI::CreateOverlay( int width, int height, const Vei2& pos, float scale/* = 1.0f*/ )
	{

		Overlay* pOverlay = overlay_allocator.AllocObject( *renderer, gpu_driver.get(), wnd, width, height, pos, scale );
		overlays.emplace_back( pOverlay );
		return pOverlay;
	}

	void BatUI::DeleteOverlay( Overlay* pOverlay )
	{
		overlay_allocator.FreeObject( pOverlay );
		overlays.erase(
			std::remove( overlays.begin(), overlays.end(), pOverlay ),
			overlays.end()
		);
	}

	void BatUI::DrawOverlays()
	{
		for( Overlay* pOverlay : overlays )
		{
			pOverlay->Draw();
		}
	}
}