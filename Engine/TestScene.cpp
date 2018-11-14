#include "TestScene.h"

#include "Window.h"
#include "Graphics.h"
#include "MathLib.h"

using namespace Bat;

TestScene::TestScene( Window& wnd, Graphics& gfx )
	:
	BaseClass( wnd, gfx )
{
	mariotex = gfx.CreateTexture( L"Assets/mario.png" );

	TexVertex v1( { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } );
	TexVertex v2( { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } );
	TexVertex v3( { 0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f } );
	TexVertex v4( { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } );

	pMario = gfx.CreateTexturedModel( { v1, v2, v3, v4 }, { 0,1,2, 2,3,0 }, mariotex );
}

void TestScene::OnUpdate()
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

void TestScene::OnRender()
{
	pMario->Draw();
}