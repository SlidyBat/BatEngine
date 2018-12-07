#include "PCH.h"
#include "ModelTestScene.h"

#include "Window.h"
#include "IGraphics.h"
#include "Camera.h"
#include "IModel.h"
#include "ModelLoader.h"
#include "LightPipeline.h"
#include "BumpMapPipeline.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "imgui.h"
#include "GenericPostProcess.h"
#include "JobSystem.h"
#include "FrameTimer.h"

using namespace Bat;

ModelTestScene::ModelTestScene( Window& wnd )
	:
	BaseClass( wnd )
{
	m_Light.SetPosition( { 0.0f, 0.0f, -5.0f } );
	m_Camera.SetPosition( 0.0f, 0.0f, -5.0f );
	g_pGfx->SetCamera( &m_Camera );
	m_Camera.SetSpeed( 20 );

	FrameTimer ft;

	std::thread t1( [this]()
	{
		m_pModel1 = std::make_unique<BumpMappedModel>( ModelLoader::LoadModel( "Assets/dodge1.FBX" ) );
	} );
	std::thread t2( [this]()
	{
		m_pModel2 = std::make_unique<BumpMappedModel>( ModelLoader::LoadModel( "Assets/dodge2.FBX" ) );
	} );
	std::thread t3( [this]()
	{
		m_pModel3 = std::make_unique<BumpMappedModel>( ModelLoader::LoadModel( "Assets/dodge3.FBX" ) );
	} );
	t1.join();
	t2.join();
	t3.join();
	
	float thread_time = ft.Mark();

	JobSystem::Execute( [this]()
	{
		m_pModel1 = std::make_unique<BumpMappedModel>( ModelLoader::LoadModel( "Assets/dodge1.FBX" ) );
	} );
	JobSystem::Execute( [this]()
	{
		m_pModel2 = std::make_unique<BumpMappedModel>( ModelLoader::LoadModel( "Assets/dodge2.FBX" ) );
	} );
	JobSystem::Execute( [this]()
	{
		m_pModel3 = std::make_unique<BumpMappedModel>( ModelLoader::LoadModel( "Assets/dodge3.FBX" ) );
	} );
	JobSystem::Wait();

	float job_time = ft.Mark();

	BAT_LOG( "Total std::thread execution time: {}", thread_time );
	BAT_LOG( "Total resource managed execution time: {}", job_time );

	m_Skybox = Texture::FromDDS( L"Assets/skybox.dds" );
	g_pGfx->SetSkybox( &m_Skybox );

	//g_pGfx->AddPostProcess( std::make_unique<GenericPostProcess>( L"Graphics/Shaders/Build/PostProcessPS.cso" ) );
}

void ModelTestScene::OnUpdate( float deltatime )
{
	m_Camera.Update( wnd.input, deltatime );

	if( wnd.input.IsKeyPressed( 'N' ) )
	{
		m_bUseBumpMap = true;
	}
	else if( wnd.input.IsKeyPressed( 'M' ) )
	{
		m_bUseBumpMap = false;
	}

	if( wnd.input.IsKeyPressed( 'C' ) )
	{
		const Vec3 pos = m_Camera.GetPosition();
		lightPos[0] = pos.x;
		lightPos[1] = pos.y;
		lightPos[2] = pos.z;
	}

	m_Light.SetPosition( { lightPos[0], lightPos[1], lightPos[2] } );
	m_Light.SetAmbient( { lightAmb[0], lightAmb[1], lightAmb[2] } );
	m_Light.SetDiffuse( { lightDiff[0], lightDiff[1], lightDiff[2] } );
	m_Light.SetSpecular( { lightSpec[0], lightSpec[1], lightSpec[2] } );
}

void ModelTestScene::OnRender()
{
	g_pGfx->EnableDepthStencil();

	LightPipeline* pPipeline;
	if( m_bUseBumpMap )
	{
		pPipeline = static_cast<LightPipeline*>(g_pGfx->GetPipeline( "bumpmap" ));
	}
	else
	{
		pPipeline = static_cast<LightPipeline*>(g_pGfx->GetPipeline( "light" ));
	}
	pPipeline->SetLight( &m_Light );

	m_pModel1->Draw( pPipeline );

	Vec3 campos = m_Camera.GetPosition();
	std::wstring pos = L"Pos: " + std::to_wstring( campos.x ) + L" " + std::to_wstring( campos.y ) + L" " + std::to_wstring( campos.z );
	g_pGfx->DrawText( pos.c_str(), DirectX::XMFLOAT2{ 15.0f, 30.0f } );

	ImGui::Begin("Light Controls");
	ImGui::SliderFloat3( "Position", lightPos, -20.0f, 20.0f );
	ImGui::SliderFloat3( "Ambient", lightAmb, 0.0f, 1.0f );
	ImGui::SliderFloat3( "Diffuse", lightDiff, 0.0f, 1.0f );
	ImGui::SliderFloat3( "Specular", lightSpec, 0.0f, 1.0f );
	ImGui::End();
}