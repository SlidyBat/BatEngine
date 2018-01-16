#include "Graphics.h"

bool Graphics::Initialize( const int screenWidth, const int screenHeight, HWND hWnd )
{
	if( !d3d.Initialize( screenWidth, screenHeight, FullScreen, hWnd, VSyncEnabled, ScreenDepth, ScreenNear ) )
	{
		MessageBox( hWnd, "Could not initialize Direct3D", "Error", MB_OK ); // should probably fix this up later to use exceptions
		return false;
	}

	camera.SetPosition( 0.0f, 0.0f, -5.0f );

	if( !colourShader.Initialize( d3d.GetDevice(), hWnd ) )
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

void Graphics::DrawTriangle( const std::array<Vertex, 3>& tri )
{
	Triangle( d3d.GetDevice(), tri ).Render( d3d.GetDeviceContext() );
	colourShader.Render( d3d.GetDeviceContext(), 3 );
}

bool Graphics::Render()
{
	d3d.BeginScene( 0.0f, 0.0f, 0.0f, 1.0f );

	Vertex v1( { -1.0f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } );
	Vertex v2( { -1.0f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } );
	Vertex v3( {  0.0f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } );
	DrawTriangle( { v1, v2, v3 } );

	Vertex v4( {  1.0f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } );
	Vertex v5( {  0.0f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } );
	Vertex v6( {  1.0f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } );
	DrawTriangle( { v4, v5, v6 } );

	d3d.EndScene();

	return true;
}
