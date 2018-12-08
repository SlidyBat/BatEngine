#include "PCH.h"
#include "MarioTestScene.h"

#include "Window.h"
#include "IGraphics.h"
#include "Camera.h"
#include "IModel.h"
#include "Material.h"
#include "TexturePipeline.h"
#include "Texture.h"
#include "ResourceManager.h"

using namespace Bat;

MarioTestScene::MarioTestScene( Window& wnd )
	:
	BaseClass( wnd )
{
	m_Camera.SetPosition( 0.0f, 0.0f, -5.0f );
	g_pGfx->SetCamera( &m_Camera );

	TexVertex v1( { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } );
	TexVertex v2( { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } );
	TexVertex v3( { 0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f } );
	TexVertex v4( { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } );

	const std::vector<Vec4> positions = {
		{ -0.5f, -0.5f, 0.0f, 1.0f },
		{ -0.5f,  0.5f, 0.0f, 1.0f },
		{ 0.5f,  0.5f, 0.0f, 1.0f },
		{ 0.5f, -0.5f, 0.0f, 1.0f }
	};

	const std::vector<Vec2> uvs = {
		{ 0.0f, 1.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f }
	};

	const std::vector<int> indices = { 0, 1, 2,  2, 3, 0 };

	Material* pMaterial = new Material();
	pMaterial->SetDiffuseTexture( ResourceManager::GetTexture( "Assets/mario.png" ) );
	MeshParameters params;
	params.position = positions;
	params.uv = uvs;
	Mesh mariomesh( params, indices, pMaterial );

	m_pMario = std::make_unique<TexturedModel>( mariomesh );
}

void MarioTestScene::OnUpdate( float deltatime )
{
	m_Camera.Update( wnd.input, deltatime );
}

void MarioTestScene::OnRender()
{
	auto pPipeline = g_pGfx->GetPipeline( "texture" );
	m_pMario->Draw( pPipeline );
}