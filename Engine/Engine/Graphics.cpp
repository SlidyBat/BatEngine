#include "Graphics.h"
#include "Vertex.h"

void Graphics::DrawLine( const std::array<TexVertex, 2>& line, Texture& texture )
{
	Line<TexVertex>( d3d.GetDevice(), line ).Render( d3d.GetDeviceContext() );
	texShader.RenderIndexed( d3d.GetDeviceContext(), 2, texture.GetTextureView() );
}

void Graphics::DrawLine( const std::array<Vertex, 2>& line )
{
	Line<Vertex>( d3d.GetDevice(), line ).Render( d3d.GetDeviceContext() );
	colShader.RenderIndexed( d3d.GetDeviceContext(), 2 );
}

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

void Graphics::DrawPoint( const std::vector<Vertex>& points )
{
	Point<Vertex>( d3d.GetDevice(), points ).Render( d3d.GetDeviceContext() );
	colShader.Render( d3d.GetDeviceContext(), points.size() );
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
}

void Graphics::EndFrame()
{
	d3d.EndScene();
}