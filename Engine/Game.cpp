#include "Game.h"
#include <chrono>

Game::Game( Window& wnd )
	:
	wnd( wnd ),
	gfx( wnd )
{
	mariotex = gfx.CreateTexture( L"Assets/mario.png" );

	TexVertex v1( { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } );
	TexVertex v2( { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } );
	TexVertex v3( { 0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f } );
	TexVertex v4( { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } );

	pMario = gfx.CreateTexturedModel( { v1, v2, v3, v4 }, { 0,1,2, 2,3,0 }, mariotex );
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
	if( wnd.input.IsKeyPressed( 'A' ) )
	{
		gfx.camera.MoveBy( -0.01f, 0.0f, 0.0f );
	}
	if( wnd.input.IsKeyPressed( 'D' ) )
	{
		gfx.camera.MoveBy( 0.01f, 0.0f, 0.0f );
	}
	if( wnd.input.IsKeyPressed( 'W' ) )
	{
		gfx.camera.MoveBy( 0.0f, 0.01f, 0.0f );
	}
	if( wnd.input.IsKeyPressed( 'S' ) )
	{
		gfx.camera.MoveBy( 0.0f, -0.01f, 0.0f );
	}
	if( wnd.input.IsKeyPressed( 'Q' ) )
	{
		gfx.camera.MoveBy( 0.0f, 0.0f, 0.01f );
	}
	if( wnd.input.IsKeyPressed( 'E' ) )
	{
		gfx.camera.MoveBy( 0.0f, 0.0f, -0.01f );
	}

	if( wnd.input.IsKeyPressed( VK_LEFT ) )
	{
		gfx.camera.RotateBy( 0.0f, -0.01f, 0.0f );
	}
	if( wnd.input.IsKeyPressed( VK_RIGHT ) )
	{
		gfx.camera.RotateBy( 0.0f, 0.01f, 0.0f );
	}
}

void Game::ComposeFrame()
{
	pMario->Draw( gfx.GetVPMatrix() );
}
