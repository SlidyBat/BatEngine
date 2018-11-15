#include "Graphics.h"
#include "VertexTypes.h"

namespace Bat
{
	IGraphics* g_pGfx = nullptr;

	Graphics::Graphics( Window& wnd )
		:
		d3d( wnd, VSyncEnabled, ScreenFar, ScreenNear )
	{
		IGraphics::RegisterGraphics( this );
		AddShader( "texture", new TextureShader( L"Graphics/Shaders/TextureVS.hlsl", L"Graphics/Shaders/TexturePS.hlsl" ) );

		m_pCamera = new Camera( FOV, (float)ScreenWidth / ScreenHeight, ScreenNear, ScreenFar );
		m_pCamera->SetPosition( 0.0f, 0.0f, -5.0f );

		wnd.AddResizeListener( [=]( int width, int height )
		{
			Resize( width, height );
			m_pCamera->SetAspectRatio( (float)width / height );
		} );
	}

	Model* Graphics::CreateTexturedModel( const std::vector<TexVertex>& vertices, const std::vector<int>& indices, Texture& tex )
	{
		TexMesh mesh( vertices, indices, &tex );
		return new TexturedModel( mesh );
	}

	Texture Graphics::CreateTexture( const std::wstring& filename )
	{
		return Texture( filename );
	}

	Texture Graphics::CreateTexture( const Colour* pPixels, int width, int height )
	{
		return Texture( pPixels, width, height );
	}

	void Graphics::BeginFrame()
	{
		d3d.BeginScene( 0.0f, 0.0f, 0.0f, 1.0f );
	}

	void Graphics::EndFrame()
	{
		m_pCamera->Render();
		d3d.EndScene();
	}
}