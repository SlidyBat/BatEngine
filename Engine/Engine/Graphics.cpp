#include "Graphics.h"
#include "Vertex.h"

Graphics::Graphics()
	:
	pPixelBuffer( std::make_unique<Colour[]>( ScreenWidth*ScreenHeight ) )
{}

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