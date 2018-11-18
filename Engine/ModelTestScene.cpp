#include "ModelTestScene.h"

#include "Window.h"
#include "IGraphics.h"
#include "Camera.h"
#include "MathLib.h"
#include "IModel.h"
#include "ModelLoader.h"
#include "LightPipeline.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "LightPipeline.h"

using namespace Bat;

ModelTestScene::ModelTestScene( Window& wnd )
	:
	BaseClass( wnd )
{
	m_Light.SetPosition( { 0.0f, 0.0f, -5.0f } );
	m_Camera.SetPosition( 0.0f, 0.0f, -5.0f );
	g_pGfx->SetCamera( &m_Camera );

	m_pNanoSuit = std::make_unique<LightModel>( ModelLoader::LoadModel( "Assets/NanoSuit/nanosuit.obj" ) );

	m_pSpriteBatch = std::make_unique<DirectX::SpriteBatch>( g_pGfx->GetDeviceContext() );
	m_pFont = std::make_unique<DirectX::SpriteFont>( g_pGfx->GetDevice(), L"Assets/Fonts/consolas.spritefont" );
}

void ModelTestScene::OnUpdate( float deltatime )
{
	m_Camera.Update( wnd.input, deltatime );
}

void ModelTestScene::OnRender()
{
	g_pGfx->EnableDepthStencil();

	auto pPipeline = static_cast<LightPipeline*>(g_pGfx->GetPipeline( "light" ));
	pPipeline->SetLight( &m_Light );

	m_pNanoSuit->Draw( pPipeline );

	m_pSpriteBatch->Begin();
	Vec3 campos = m_Camera.GetPosition();
	std::wstring pos = L"Pos: " + std::to_wstring( campos.x ) + L" " + std::to_wstring( campos.y ) + L" " + std::to_wstring( campos.z );
	m_pFont->DrawString( m_pSpriteBatch.get(), pos.c_str(), DirectX::XMFLOAT2{ 15.0f, 30.0f } );
	m_pSpriteBatch->End();
}