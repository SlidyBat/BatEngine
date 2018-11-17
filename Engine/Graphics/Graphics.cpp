#include "Graphics.h"
#include "VertexTypes.h"
#include "TexturePipeline.h"
#include "LightPipeline.h"
#include "ColourPipeline.h"
#include "IModel.h"
#include "Window.h"

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
	}

	void Graphics::EndFrame()
	{
		m_pCamera->Render();
		d3d.EndScene();
	}
}