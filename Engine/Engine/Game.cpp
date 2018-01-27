#include "Game.h"
#include <chrono>

bool Game::Initialize( int screenWidth, int screenHeight, HWND hWnd )
{
	return gfx.Initialize( screenWidth, screenHeight, hWnd );
}

void Game::Run()
{
	gfx.BeginFrame();
	UpdateModels();
	ComposeFrame();
	gfx.EndFrame();
}

void Game::UpdateModels()
{
	drawMario = Input::IsKeyPressed( VK_SPACE );
}

void Game::ComposeFrame()
{
	gfx.DrawLine( std::array<Vertex, 2> {
		Vertex{ { -0.95f, -0.95f, 0.0f },{ 1.0f, 1.0f, 0.0f, 1.0f } },
			Vertex{ { 0.95f, 0.95f, 0.0f },{ 0.0f, 1.0f, 1.0f, 1.0f } } } );

	static Texture mario = gfx.CreateTexture( L"mario.png" );
	TexVertex v1( { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } );
	TexVertex v2( { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } );
	TexVertex v3( { 0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f } );
	TexVertex v4( { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } );
	gfx.DrawQuad( { v1, v2, v3, v4 }, mario );
}
