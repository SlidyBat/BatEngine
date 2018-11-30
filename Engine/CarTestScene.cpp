#include "BatWinAPI.h"
#include "CarTestScene.h"

#include "Window.h"
#include "IGraphics.h"
#include "Camera.h"
#include "MathLib.h"
#include "IModel.h"
#include "ModelLoader.h"
#include "BumpMapPipeline.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "imgui.h"
#include "GenericPostProcess.h"

using namespace Bat;

CarTestScene::CarTestScene( Window& wnd )
	:
	BaseClass( wnd ),
	m_Camera( 100.0f )
{
	m_Light.SetPosition( { 0.0f, 0.0f, -5.0f } );
	m_Camera.SetPosition( 0.0f, 0.0f, -5.0f );
	g_pGfx->SetCamera( &m_Camera );

	m_pCar = std::make_unique<BumpMappedModel>( ModelLoader::LoadModel( "Assets/sword/scene.gltf" ) );

	m_Skybox = Texture::FromDDS( L"Assets/skybox.dds" );
	g_pGfx->SetSkybox( &m_Skybox );

	//g_pGfx->AddPostProcess( std::make_unique<GenericPostProcess>( L"Graphics/Shaders/Build/PostProcessPS.cso" ) );

}

void CarTestScene::OnUpdate( float deltatime )
{
	m_Light.SetPosition( { lightPos[0], lightPos[1], lightPos[2] } );
	m_Light.SetAmbient( { lightAmb[0], lightAmb[1], lightAmb[2] } );
	m_Light.SetDiffuse( { lightDiff[0], lightDiff[1], lightDiff[2] } );
	m_Light.SetSpecular( { lightSpec[0], lightSpec[1], lightSpec[2] } );
	m_Camera.Update( wnd.input, deltatime );
}

void CarTestScene::OnRender()
{
	g_pGfx->EnableDepthStencil();

	auto pPipeline = dynamic_cast<LightPipeline*>(g_pGfx->GetPipeline( "bumpmap" ));
	pPipeline->SetLight( &m_Light );

	m_pCar->Draw( pPipeline );

	Vec3 campos = m_Camera.GetPosition();
	std::wstring pos = L"Pos: " + std::to_wstring( campos.x ) + L" " + std::to_wstring( campos.y ) + L" " + std::to_wstring( campos.z );
	g_pGfx->DrawText( pos.c_str(), { 15.0f, 30.0f } );

	ImGui::Begin( "Light Controls" );
	ImGui::SliderFloat3( "Position", lightPos, -1000.0f, 1000.0f );
	ImGui::SliderFloat3( "Ambient", lightAmb, 0.0f, 1.0f );
	ImGui::SliderFloat3( "Diffuse", lightDiff, 0.0f, 1.0f );
	ImGui::SliderFloat3( "Specular", lightSpec, 0.0f, 1.0f );
	ImGui::End();
}