#include "Graphics.h"
#include "Vertex.h"

Graphics::Graphics()
	:
	pPixelBuffer( std::make_unique<Colour[]>( ScreenWidth*ScreenHeight ) )
{}

bool Graphics::Initialize( const int screenWidth, const int screenHeight, HWND hWnd )
{
	if( !d3d.Initialize( screenWidth, screenHeight, FullScreen, hWnd, VSyncEnabled, ScreenDepth, ScreenNear ) )
	{
		MessageBox( hWnd, "Could not initialize Direct3D", "Error", MB_OK ); // should probably fix this up later to use exceptions
		return false;
	}

	camera.SetPosition( 0.0f, 0.0f, -5.0f );

	if( !shader.Initialize( d3d.GetDevice(), hWnd, L"Texture.vs", L"Texture.ps" ) )
	{
		return false;
	}

	return true;
}

bool Graphics::Frame()
{
	if( !Render() )
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

bool Graphics::Render()
{
	BeginFrame();

	DrawPixel( 50, 50, { 255, 255, 255 } );
	DrawPixel( 50, 51, { 255, 0,   0   } );
	DrawPixel( 50, 52, { 0,   255, 0   } );
	DrawPixel( 50, 53, { 0,   0,   255 } );
	
	static Texture mario( d3d.GetDevice(), d3d.GetDeviceContext(), L"mario.png" );

	TexVertex v1( { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } );
	TexVertex v2( { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } );
	TexVertex v3( {  0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f } );
	TexVertex v4( {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } );
	DrawQuad( std::array<TexVertex, 4>{ v1, v2, v3, v4 }, mario );

	EndFrame();

	return true;
}
