#pragma once

#include "Game.h"
#include "Window.h"
#include "Graphics.h"
#include "Input.h"

class Game
{
public:
	Game( Window& wnd );
	Game( const Game& src ) = delete;
	Game& operator=( const Game& src ) = delete;
	Game( Game&& donor ) = delete;
	Game& operator=( Game&& donor ) = delete;

	void Run();
private:
	void UpdateModels();
	void ComposeFrame();
private:
	Window& wnd;
	Graphics gfx;
};