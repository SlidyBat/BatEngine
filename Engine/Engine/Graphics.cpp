#include "Graphics.h"
#include "Vertex.h"

Graphics::Graphics()
	:
	pPixelBuffer( std::make_unique<Colour[]>( ScreenWidth*ScreenHeight ) )
{}

void Graphics::DrawTriangle( const std::array<TexVertex, 3>& tri, Texture& texture )
{
	Triangle<TexVertex>( d3d.GetDevice(), tri ).Render( d3d.GetDeviceContext() );
	texShader.RenderIndexed( d3d.GetDeviceContext(), Triangle<TexVertex>::GetIndexCount(), texture.GetTextureView() );
}

void Graphics::DrawTriangle( const std::array<Vertex, 3>& tri )
{
	Triangle<Vertex>( d3d.GetDevice(), tri ).Render( d3d.GetDeviceContext() );
	colShader.RenderIndexed( d3d.GetDeviceContext(), Triangle<Vertex>::GetIndexCount() );
}

void Graphics::DrawQuad( const std::array<TexVertex, 4>& quad, Texture & texture )
{
	Quad<TexVertex>( d3d.GetDevice(), quad ).Render( d3d.GetDeviceContext() );
	texShader.RenderIndexed( d3d.GetDeviceContext(), Quad<TexVertex>::GetIndexCount(), texture.GetTextureView() );
}

void Graphics::DrawQuad( const std::array<Vertex, 4>& quad )
{
	Quad<Vertex>( d3d.GetDevice(), quad ).Render( d3d.GetDeviceContext() );
	colShader.RenderIndexed( d3d.GetDeviceContext(), Quad<Vertex>::GetIndexCount() );
}

void Graphics::DrawPixel( const int x, const int y, const Colour & c )
{
	pPixelBuffer[x + y*ScreenWidth] = c;

	static const std::array<TexVertex, 4> screenQuad =
	{
		TexVertex{ { -1.0f, -1.0f, 0.0f },{ 0.0f, 1.0f } },
		TexVertex{ { -1.0f,  1.0f, 0.0f },{ 0.0f, 0.0f } },
		TexVertex{ { 1.0f,  1.0f, 0.0f },{ 1.0f, 1.0f } },
		TexVertex{ { 1.0f, -1.0f, 0.0f },{ 1.0f, 0.0f } }
	};

	DrawQuad( screenQuad, Texture( d3d.GetDevice(), d3d.GetDeviceContext(), pPixelBuffer.get(), ScreenWidth, ScreenHeight ) );
}

Texture Graphics::CreateTexture( const std::wstring& filename )
{
	return Texture( d3d.GetDevice(), d3d.GetDeviceContext(), filename );
}

Texture Graphics::CreateTexture( const Colour* pPixels, int width, int height )
{
	return Texture( d3d.GetDevice(), d3d.GetDeviceContext(), pPixels, width, height );
}

bool Graphics::Initialize( const int screenWidth, const int screenHeight, HWND hWnd )
{
	if( !d3d.Initialize( screenWidth, screenHeight, FullScreen, hWnd, VSyncEnabled, ScreenDepth, ScreenNear ) )
	{
		MessageBox( hWnd, "Could not initialize Direct3D", "Error", MB_OK ); // should probably fix this up later to use exceptions
		return false;
	}

	camera.SetPosition( 0.0f, 0.0f, -5.0f );

	if( !colShader.Initialize( d3d.GetDevice(), hWnd, L"Colour.vs", L"Colour.ps" ) )
	{
		return false;
	}

	if( !texShader.Initialize( d3d.GetDevice(), hWnd, L"Texture.vs", L"Texture.ps" ) )
	{
		return false;
	}

	return true;
}

void Graphics::BeginFrame()
{
	d3d.BeginScene( 0.0f, 0.0f, 0.0f, 1.0f );
	memset( pPixelBuffer.get(), 0, ScreenWidth*ScreenHeight );
}

void Graphics::EndFrame()
{
	d3d.EndScene();
}