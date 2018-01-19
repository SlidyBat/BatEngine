#include "Game.h"

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
	gfx.DrawPixel( 50, 50, { 255, 255, 255 } );
	gfx.DrawPixel( 50, 51, { 255, 0,   0   } );
	gfx.DrawPixel( 50, 52, { 0,   255, 0   } );
	gfx.DrawPixel( 50, 53, { 0,   0,   255 } );

	if( drawMario )
	{
		static Texture mario = gfx.CreateTexture( L"mario.png" );
		TexVertex v1( { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } );
		TexVertex v2( { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } );
		TexVertex v3( {  0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f } );
		TexVertex v4( {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } );
		gfx.DrawQuad( { v1, v2, v3, v4 }, mario );
	}
}
