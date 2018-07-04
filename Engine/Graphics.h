#pragma once

#include "SlidyWin.h"
#include "D3DClass.h"

#include "Window.h"
#include "GDIPManager.h"
#include "Colour.h"
#include "Texture.h"
#include "Vertex.h"
#include "TexVertex.h"
#include "Camera.h"
#include "TextureShader.h"
#include "ColourShader.h"
#include "Point.h"
#include "Line.h"
#include "Triangle.h"
#include "Quad.h"

#include <memory>

class Graphics
{
public:
	Graphics( Window& wnd );
	Graphics( const Graphics& src ) = delete;
	Graphics& operator=( const Graphics& src ) = delete;
	Graphics( Graphics&& donor ) = delete;
	Graphics& operator=( Graphics&& donor ) = delete;

	void DrawPoint( const std::vector<Vertex>& points );
	void DrawPoint( const Vertex& point )
	{
		DrawPoint( std::vector<Vertex>{ point } );
	}
	void DrawLine( const std::array<TexVertex, 2>& line, Texture& texture );
	void DrawLine( const std::array<Vertex, 2>& line );
	void DrawTriangle( const std::array<TexVertex, 3>& tri, Texture& texture );
	void DrawTriangle( const std::array<Vertex, 3>& tri );
	void DrawQuad( const std::array<TexVertex, 4>& quad, Texture& texture );
	void DrawQuad( const std::array<Vertex, 4>& quad );
	

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