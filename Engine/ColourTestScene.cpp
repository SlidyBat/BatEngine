#include "PCH.h"
#include "ColourTestScene.h"

#include "Window.h"
#include "IGraphics.h"
#include "Camera.h"
#include "IModel.h"
#include "ColourPipeline.h"

using namespace Bat;

ColourTestScene::ColourTestScene( Window& wnd )
	:
	BaseClass( wnd )
{
	m_Camera.SetPosition( 0.0f, 0.0f, -5.0f );
	g_pGfx->SetCamera( &m_Camera );
	Vec4 red( 1.0f, 0.0f, 0.0f, 1.0f );

	const std::vector<Vec4> positions = {
		{ -0.5f, -0.5f, 0.0f, 1.0f },
		{ -0.5f,  0.5f, 0.0f, 1.0f },
		{ 0.5f,  0.5f, 0.0f, 1.0f },
		{ 0.5f, -0.5f, 0.0f, 1.0f }
	};

	const std::vector<Vec4> colours = {
		red, red, red, red
	};
	
	const std::vector<int> indices = { 0, 1, 2,  2, 3, 0 };

	MeshParameters params;
	params.position = positions;
	params.colour = colours;

	Mesh redmesh( params, indices, nullptr );
	m_pRedSquare = std::make_unique<ColouredModel>( redmesh );
}

void ColourTestScene::OnUpdate( float deltatime )
{
	m_Camera.Update( wnd.input, deltatime );
}

void ColourTestScene::OnRender()
{
	auto pPipeline = g_pGfx->GetPipeline( "colour" );
	m_pRedSquare->Draw( pPipeline );
}