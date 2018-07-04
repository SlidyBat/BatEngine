#pragma once

#include "SlidyWin.h"
#include "D3DClass.h"

#include "Window.h"
#include "GDIPManager.h"
#include "Colour.h"
#include "Texture.h"
#include "Vertex.h"
#include "TexVertex.h"
#include "Model.h"
#include "Camera.h"
#include "TextureShader.h"
#include "ColourShader.h"


#include <memory>

class Graphics
{
public:
	Graphics( Window& wnd );
	Graphics( const Graphics& src ) = delete;
	Graphics& operator=( const Graphics& src ) = delete;
	Graphics( Graphics&& donor ) = delete;
	Graphics& operator=( Graphics&& donor ) = delete;

	void Resize( int width = 0, int height = 0 )
	{
		d3d.Resize( width, height );
	}

	void DrawModel( std::vector<TexVertex> vertices, std::vector<int> indices, const Texture& texture, D3D_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	void DrawModel( std::vector<TexVertex> vertices, const Texture& texture, D3D_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	void DrawModel( std::vector<Vertex> vertices, std::vector<int> indices, D3D_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	void DrawModel( std::vector<Vertex> vertices, D3D_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	void DrawPoints( const std::vector<Vertex>& points );
	void DrawPoint( const Vertex& point )
	{
		DrawPoints( std::vector<Vertex>{ point } );
	}
	void DrawLine( const TexVertex& p1, const TexVertex& p2, const Texture& texture );
	void DrawLine( const Vertex& p1, const Vertex& p2 );
	void DrawTriangle( const TexVertex& p1, const TexVertex& p2, const TexVertex& p3, const Texture& texture );
	void DrawTriangle( const Vertex& p1, const Vertex& p2, const Vertex& p3 );
	void DrawQuad( const TexVertex& p1, const TexVertex& p2, const TexVertex& p3, const TexVertex& p4, const Texture& texture );
	void DrawQuad( const Vertex& p1, const Vertex& p2, const Vertex& p3, const Vertex& p4 );

	Texture CreateTexture( const std::wstring& filename );
	Texture CreateTexture( const Colour* pPixels, int width, int height );

	void BeginFrame();
	void EndFrame();
private:
	D3DClass d3d;

	Camera camera;
	TextureShader texShader;
	ColourShader colShader;

	GDIPManager gdip;
public:
	static constexpr bool	FullScreen = false;
	static constexpr int	VSyncEnabled = false;
	static constexpr float	ScreenDepth = 1000.0f;
	static constexpr float	ScreenNear = 0.1f;

	// only used when not in fullscreen
	static constexpr int	ScreenWidth = 800;
	static constexpr int	ScreenHeight = 600;
};