#include "ModelTestScene.h"

#include "Window.h"
#include "IGraphics.h"
#include "Camera.h"
#include "MathLib.h"
#include "IModel.h"
#include "ModelLoader.h"
#include "LightPipeline.h"

using namespace Bat;

ModelTestScene::ModelTestScene( Window& wnd )
	:
	BaseClass( wnd )
{
	pNanoSuit = new LightModel( ModelLoader::LoadModel( "Assets/NanoSuit/nanosuit.obj" ) );
}

void ModelTestScene::OnUpdate()
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

	if( wnd.input.IsKeyPressed( VK_SPACE ) )
	{
		pCamera->MoveBy( 0.0f, 0.01f, 0.0f );
	}
	if( wnd.input.IsKeyPressed( VK_SHIFT ) )
	{
		pCamera->MoveBy( 0.0f, -0.01f, 0.0f );
	}
}

void ModelTestScene::OnRender()
{
	auto pPipeline = g_pGfx->GetPipeline( "light" );
	pNanoSuit->Draw( pPipeline );
}