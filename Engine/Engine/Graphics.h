#pragma once

#include "SlidyWin.h"
#include "D3DClass.h"

#include "Texture.h"
#include "TexVertex.h"
#include "Camera.h"
#include "TextureShader.h"
#include "Triangle.h"

class Graphics
{
public:
	Graphics() = default;
	Graphics( const Graphics& src ) = delete;
	Graphics& operator=( const Graphics& src ) = delete;
	Graphics( Graphics&& donor ) = delete;
	Graphics& operator=( Graphics&& donor ) = delete;

	void DrawTriangle( const std::array<TexVertex, 3>& tri, Texture& texture )
	{
		Triangle<TexVertex>( d3d.GetDevice(), tri ).Render( d3d.GetDeviceContext() );
		shader.Render( d3d.GetDeviceContext(), 3, texture.GetTextureView() );
	}
	void DrawQuad( const std::array<TexVertex, 4>& quad, Texture& texture )
	{
		Triangle<TexVertex>( d3d.GetDevice(), { quad[0], quad[1], quad[2] } ).Render( d3d.GetDeviceContext() );
		shader.Render( d3d.GetDeviceContext(), 3, texture.GetTextureView() );
		Triangle<TexVertex>( d3d.GetDevice(), { quad[2], quad[3], quad[0] } ).Render( d3d.GetDeviceContext() );
		shader.Render( d3d.GetDeviceContext(), 3, texture.GetTextureView() );
	}

	bool Initialize( const int screenWidth, const int screenHeight, HWND hWnd );
	bool Frame();
private:
	bool Render();
private:
	D3DClass d3d;

	Camera camera;
	TextureShader shader;
public:
	static constexpr bool FullScreen = false;
	static constexpr int VSyncEnabled = false;
	static constexpr float ScreenDepth = 1000.0f;
	static constexpr float ScreenNear = 0.1f;

	// only used when not in fullscreen
	static constexpr int ScreenWidth = 1000;
	static constexpr int ScreenHeight = 500;
};