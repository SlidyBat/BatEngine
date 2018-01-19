#pragma once

#include "Game.h"
#include "Graphics.h"
#include "Input.h"

class Game
{
public:
	Game() = default;
	Game( const Game& src ) = delete;
	Game& operator=( const Game& src ) = delete;
	Game( Game&& donor ) = delete;
	Game& operator=( Game&& donor ) = delete;

	bool Initialize( int screenWidth, int screenHeight, HWND hWnd );
	void Run();
private:
	void UpdateModels();
	void ComposeFrame();
private:
	Graphics gfx;

	bool drawMario = false;
};