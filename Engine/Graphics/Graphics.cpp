#include "Graphics.h"
#include "Vertex.h"

Graphics::Graphics( Window& wnd )
	:
	d3d( wnd, VSyncEnabled, ScreenFar, ScreenNear ),
	colShader( d3d.GetDevice(), wnd.GetHandle(), L"Graphics/Shaders/ColourVS.hlsl", L"Graphics/Shaders/ColourPS.hlsl" ),
	texShader( d3d.GetDevice(), L"Graphics/Shaders/TextureVS.hlsl", L"Graphics/Shaders/TexturePS.hlsl" ),
	camera( FOV, (float)ScreenWidth / ScreenHeight, ScreenNear, ScreenFar )
{
	camera.SetPosition( 0.0f, 0.0f, -5.0f );

	projection = DirectX::XMMatrixPerspectiveFovLH( FOVRadians, (float)wnd.GetWidth() / wnd.GetHeight(), ScreenNear, ScreenFar );

	wnd.AddResizeListener( [=]( int width, int height )
	{
		Resize( width, height );
		camera.SetAspectRatio( (float)width / height );
	} );
}

Model* Graphics::CreateTexturedModel( const std::vector<TexVertex>& vertices, const std::vector<int>& indices, Texture& tex )
{
	return new TexturedModel( d3d.GetDevice(), d3d.GetDeviceContext(), &texShader, &camera, &tex, vertices, indices );
}

Texture Graphics::CreateTexture( const std::wstring& filename )
{
	return Texture( d3d.GetDevice(), d3d.GetDeviceContext(), filename );
}

Texture Graphics::CreateTexture( const Colour* pPixels, int width, int height )
{
	return Texture( d3d.GetDevice(), d3d.GetDeviceContext(), pPixels, width, height );
}

void Graphics::BeginFrame()
{
	d3d.BeginScene( 0.0f, 0.0f, 0.0f, 1.0f );
}

void Graphics::EndFrame()
{
	d3d.EndScene();
}