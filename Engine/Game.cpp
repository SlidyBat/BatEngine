#include "Game.h"

#include "MathLib.h"

namespace Bat
{
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
		Vec3 forward = gfx.camera.GetForwardVector();
		Vec3 right = gfx.camera.GetRightVector();

		if( wnd.input.IsKeyPressed( 'A' ) )
		{
			gfx.camera.MoveBy( -right * 0.01f );
		}
		if( wnd.input.IsKeyPressed( 'D' ) )
		{
			gfx.camera.MoveBy( right * 0.01f );
		}
		if( wnd.input.IsKeyPressed( 'W' ) )
		{
			gfx.camera.MoveBy( forward * 0.01f );
		}
		if( wnd.input.IsKeyPressed( 'S' ) )
		{
			gfx.camera.MoveBy( -forward * 0.01f );
		}

		if( wnd.input.IsKeyPressed( VK_UP ) )
		{
			gfx.camera.RotateBy( -0.1f, 0.0f, 0.0f );
		}
		if( wnd.input.IsKeyPressed( VK_DOWN ) )
		{
			gfx.camera.RotateBy( 0.1f, 0.0f, 0.0f );
		}
		if( wnd.input.IsKeyPressed( VK_LEFT ) )
		{
			gfx.camera.RotateBy( 0.0f, -0.1f, 0.0f );
		}
		if( wnd.input.IsKeyPressed( VK_RIGHT ) )
		{
			gfx.camera.RotateBy( 0.0f, 0.1f, 0.0f );
		}
	}

	void Game::ComposeFrame()
	{
		pMario->Draw();
	}
}