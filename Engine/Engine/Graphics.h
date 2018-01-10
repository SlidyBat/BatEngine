#pragma once

#include "SlidyWin.h"
#include "D3DClass.h"

class Graphics
{
public:
	Graphics() = default;
	Graphics( const Graphics& src ) = delete;
	Graphics& operator=( const Graphics& src ) = delete;
	Graphics( Graphics&& donor ) = delete;
	Graphics& operator=( Graphics&& donor ) = delete;

	bool Initialize( const int screenWidth, const int screenHeight, HWND hWnd );
	bool Frame();
private:
	bool Render();
private:
	D3DClass d3d;
public:
	static constexpr bool FullScreen = false;
	static constexpr int VSyncEnabled = true;
	static constexpr float ScreenDepth = 1000.0f;
	static constexpr float ScreenNear = 0.1f;

	// only used when not in fullscreen
	static constexpr int ScreenWidth = 1000;
	static constexpr int ScreenHeight = 500;
};