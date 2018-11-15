#include "MarioTestScene.h"

#include "Window.h"
#include "IGraphics.h"
#include "Camera.h"
#include "MathLib.h"
#include "IModel.h"

using namespace Bat;

MarioTestScene::MarioTestScene( Window& wnd )
	:
	BaseClass( wnd )
{
	pMarioTex = g_pGfx->CreateTexture( L"Assets/mario.png" );

	TexVertex v1( { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } );
	TexVertex v2( { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } );
	TexVertex v3( { 0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f } );
	TexVertex v4( { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } );

	pMario = g_pGfx->CreateTexturedModel( { v1, v2, v3, v4 }, { 0,1,2, 2,3,0 }, *pMarioTex );
}

void MarioTestScene::OnUpdate( float deltatime )
{
	Camera* pCamera = g_pGfx->GetCamera();

	Vec3 forward = pCamera->GetForwardVector();
	Vec3 right = pCamera->GetRightVector();

	if( wnd.input.IsKeyPressed( 'A' ) )
	{
		pCamera->MoveBy( -right * 0.01f );
	}
	if( wnd.input.IsKeyPressed( 'D' ) )
	{
		pCamera->MoveBy( right * 0.01f );
	}
	if( wnd.input.IsKeyPressed( 'W' ) )
	{
		pCamera->MoveBy( forward * 0.01f );
	}
	if( wnd.input.IsKeyPressed( 'S' ) )
	{
		pCamera->MoveBy( -forward * 0.01f );
	}

	if( wnd.input.IsKeyPressed( VK_UP ) )
	{
		pCamera->RotateBy( -0.1f, 0.0f, 0.0f );
	}
	if( wnd.input.IsKeyPressed( VK_DOWN ) )
	{
		pCamera->RotateBy( 0.1f, 0.0f, 0.0f );
	}
	if( wnd.input.IsKeyPressed( VK_LEFT ) )
	{
		pCamera->RotateBy( 0.0f, -0.1f, 0.0f );
	}
	if( wnd.input.IsKeyPressed( VK_RIGHT ) )
	{
		pCamera->RotateBy( 0.0f, 0.1f, 0.0f );
	}
}

void MarioTestScene::OnRender()
{
	auto pPipeline = g_pGfx->GetPipeline( "texture" );
	pMario->Draw( pPipeline );
}