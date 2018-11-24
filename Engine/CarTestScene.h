#pragma once

#pragma once

#include "IScene.h"
#include "Texture.h"
#include "ClassDef.h"
#include "MoveableCamera.h"
#include "Light.h"

namespace Bat
{
	class Window;
	class IModel;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

class CarTestScene : public Bat::IScene
{
	DECLARE_CLASS( CarTestScene, IScene );
public:
	CarTestScene( Bat::Window& wnd );

	virtual void OnUpdate( float deltatime ) override;
	virtual void OnRender() override;
private:
	std::unique_ptr<Bat::IModel> m_pCar;
	Bat::MoveableCamera m_Camera;

	std::unique_ptr<DirectX::SpriteBatch> m_pSpriteBatch;
	std::unique_ptr<DirectX::SpriteFont> m_pFont;

	Bat::Light m_Light;

	float lightPos[3] = { -300.0f, 370.0f, 300.0f };
	float lightAmb[3] = { 0.5f, 0.5f, 0.5f };
	float lightDiff[3] = { 0.5f, 0.5f, 0.5f };
	float lightSpec[3] = { 1.0f, 1.0f, 1.0f };
};