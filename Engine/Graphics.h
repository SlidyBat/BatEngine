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

	Model* CreateTexturedModel( const std::vector<TexVertex>& vertices, const std::vector<int>& indices, Texture& tex );

	Texture CreateTexture( const std::wstring& filename );
	Texture CreateTexture( const Colour* pPixels, int width, int height );

	void BeginFrame();
	void EndFrame();

	DirectX::XMMATRIX GetVPMatrix()
	{
		camera.Render();
		return camera.GetViewMatrix() * projection;
	}
private:
	D3DClass d3d;

	TextureShader texShader;
	ColourShader colShader;

	GDIPManager gdip;

	DirectX::XMMATRIX projection;
public:
	static constexpr bool	FullScreen = false;
	static constexpr int	VSyncEnabled = false;
	static constexpr float	ScreenFar = 1000.0f;
	static constexpr float	ScreenNear = 0.1f;
	static constexpr float	FOV = 90.0f;
	static constexpr float  FOVRadians = FOV * (DirectX::XM_PI / 180.0f);

	// only used when not in fullscreen
	static constexpr int	ScreenWidth = 800;
	static constexpr int	ScreenHeight = 600;

	Camera camera;
};