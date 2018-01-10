#include "Graphics.h"

bool Graphics::Initialize( const int screenWidth, const int screenHeight, HWND hWnd )
{
	if( !d3d.Initialize( screenWidth, screenHeight, FullScreen, hWnd, VSyncEnabled, ScreenDepth, ScreenNear ) )
	{
		MessageBox( hWnd, "Could not initialize Direct3D", "Error", MB_OK );
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
	d3d.BeginScene( 1.0f, 1.0f, 0.0f, 1.0f );

	d3d.EndScene();

	return true;
}
