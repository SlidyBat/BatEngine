#pragma once

#include "SlidyWin.h"
#include "D3DClass.h"

#include "Colour.h"
#include "Texture.h"
#include "Vertex.h"
#include "TexVertex.h"
#include "Camera.h"
#include "TextureShader.h"
#include "ColourShader.h"
#include "Point.h"
#include "Triangle.h"
#include "Quad.h"

#include <memory>

class Graphics
{
public:
	Graphics() = default;
	Graphics( const Graphics& src ) = delete;
	Graphics& operator=( const Graphics& src ) = delete;
	Graphics( Graphics&& donor ) = delete;
	Graphics& operator=( Graphics&& donor ) = delete;

	void DrawPoint( const std::vector<Vertex>& points );
	void DrawPoint( const Vertex& point )
	{
		DrawPoint( std::vector<Vertex>{ point } );
	}
	void DrawTriangle( const std::array<TexVertex, 3>& tri, Texture& texture );
	void DrawTriangle( const std::array<Vertex, 3>& tri );
	void DrawQuad( const std::array<TexVertex, 4>& quad, Texture& texture );
	void DrawQuad( const std::array<Vertex, 4>& quad );
	

	Texture CreateTexture( const std::wstring& filename );
	Texture CreateTexture( const Colour* pPixels, int width, int height );

	bool Initialize( const int screenWidth, const int screenHeight, HWND hWnd );

	void BeginFrame();
	void EndFrame();
private:
	D3DClass d3d;

	Camera camera;
	TextureShader texShader;
	ColourShader colShader;
public:
	static constexpr bool FullScreen = false;
	static constexpr int VSyncEnabled = false;
	static constexpr float ScreenDepth = 1000.0f;
	static constexpr float ScreenNear = 0.1f;

	// only used when not in fullscreen
	static constexpr int ScreenWidth = 1000;
	static constexpr int ScreenHeight = 500;
};