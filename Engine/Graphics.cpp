#include "Graphics.h"
#include "Vertex.h"

Graphics::Graphics( Window& wnd )
	:
	d3d( wnd, VSyncEnabled, ScreenFar, ScreenNear ),
	colShader( d3d.GetDevice(), wnd.GetHandle(), L"ColourVS.hlsl", L"ColourPS.hlsl" ),
	texShader( d3d.GetDevice(), wnd.GetHandle(), L"TextureVS.hlsl", L"TexturePS.hlsl" )
{
	camera.SetPosition( 0.0f, 0.0f, -5.0f );

	projection = DirectX::XMMatrixPerspectiveFovLH( FOVRadians, (float)wnd.GetWidth() / wnd.GetHeight(), ScreenNear, ScreenFar );

	wnd.AddResizeListener( [=]( int width, int height )
	{
		Resize();
		projection = DirectX::XMMatrixPerspectiveFovLH( FOVRadians, (float)width / height, ScreenNear, ScreenFar );
	} );
}

void Graphics::DrawModel( std::vector<TexVertex> vertices, std::vector<int> indices, const Texture& texture, D3D_PRIMITIVE_TOPOLOGY topology )
{
	Model<TexVertex> model( d3d.GetDevice(), vertices, indices, topology );
	model.Render( d3d.GetDeviceContext() );

	texShader.RenderIndexed( d3d.GetDeviceContext(), model.GetIndexCount(), texture.GetTextureView(), GetWVP() );
}

void Graphics::DrawModel( std::vector<TexVertex> vertices, const Texture& texture, D3D_PRIMITIVE_TOPOLOGY topology )
{
	Model<TexVertex> model( d3d.GetDevice(), vertices, {}, topology );
	model.Render( d3d.GetDeviceContext() );

	texShader.Render( d3d.GetDeviceContext(), model.GetVertexCount(), texture.GetTextureView(), GetWVP() );
}

void Graphics::DrawModel( std::vector<Vertex> vertices, std::vector<int> indices, D3D_PRIMITIVE_TOPOLOGY topology )
{
	Model<Vertex> model( d3d.GetDevice(), vertices, indices, topology );
	model.Render( d3d.GetDeviceContext() );

	colShader.RenderIndexed( d3d.GetDeviceContext(), model.GetIndexCount(), GetWVP() );
}

void Graphics::DrawModel( std::vector<Vertex> vertices, D3D_PRIMITIVE_TOPOLOGY topology )
{
	Model<Vertex> model( d3d.GetDevice(), vertices, {}, topology );
	model.Render( d3d.GetDeviceContext() );

	colShader.Render( d3d.GetDeviceContext(), model.GetVertexCount(), GetWVP() );
}

void Graphics::DrawPoints( const std::vector<Vertex>& points )
{
	DrawModel( points, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
}

void Graphics::DrawLine( const TexVertex& p1, const TexVertex& p2, const Texture& texture )
{
	DrawModel( { p1, p2 }, texture, D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
}

void Graphics::DrawLine( const Vertex& p1, const Vertex& p2 )
{
	DrawModel( { p1, p2 }, D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
}

void Graphics::DrawTriangle( const TexVertex & p1, const TexVertex & p2, const TexVertex & p3, const Texture & texture )
{
	DrawModel( { p1, p2, p3 }, texture );
}

void Graphics::DrawTriangle( const Vertex& p1, const Vertex& p2, const Vertex& p3 )
{
	DrawModel( { p1, p2, p3 } );
}

void Graphics::DrawQuad( const TexVertex& p1, const TexVertex& p2, const TexVertex& p3, const TexVertex& p4, const Texture& texture )
{
	static std::vector<int> indices = {
		0, 1, 2,
		2, 3, 0
	};

	DrawModel( { p1, p2, p3, p4 }, indices, texture );
}

void Graphics::DrawQuad( const Vertex& p1, const Vertex& p2, const Vertex& p3, const Vertex& p4 )
{
	static std::vector<int> indices = {
		0, 1, 2,
		2, 3, 0
	};

	DrawModel( { p1, p2, p3, p4 }, indices );
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