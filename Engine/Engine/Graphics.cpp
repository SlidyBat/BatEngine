#include "Graphics.h"
#include "Vertex.h"

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

bool Graphics::Render()
{
	d3d.BeginScene( 0.0f, 0.0f, 0.0f, 1.0f );

	static Texture mario( d3d.GetDevice(), d3d.GetDeviceContext(), L"mario.png" );

	TexVertex v1( { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } );
	TexVertex v2( { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } );
	TexVertex v3( {  0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f } );
	TexVertex v4( { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } );
	DrawQuad( std::array<TexVertex, 4>{ v1, v2, v3, v4 }, mario );

	d3d.EndScene();

	return true;
}
