#include "Graphics.h"

bool Graphics::Initialize( const int screenWidth, const int screenHeight, HWND hWnd )
{
	if( !d3d.Initialize( screenWidth, screenHeight, FullScreen, hWnd, VSyncEnabled, ScreenDepth, ScreenNear ) )
	{
		MessageBox( hWnd, "Could not initialize Direct3D", "Error", MB_OK ); // should probably fix this up later to use exceptions
		return false;
	}

	camera.SetPosition( 0.0f, 0.0f, -5.0f );

	if( !model.Initialize( d3d.GetDevice() ) )
	{
		MessageBox( hWnd, "Failed to initialize model", "Error", MB_OK );
		return false;
	}

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

bool Graphics::Render()
{
	d3d.BeginScene( 0.0f, 0.0f, 0.0f, 1.0f );

	camera.Render();

	DirectX::XMMATRIX worldMat		= d3d.GetWorldMatrix();
	DirectX::XMMATRIX viewMat		= camera.GetViewMatrix();
	DirectX::XMMATRIX projectionMat = d3d.GetProjectionMatrix();

	model.Render( d3d.GetDeviceContext() );
	
	if( !colourShader.Render( d3d.GetDeviceContext(), model.GetIndexCount(), worldMat, viewMat, projectionMat ) )
	{
		return false;
	}

	d3d.EndScene();

	return true;
}
