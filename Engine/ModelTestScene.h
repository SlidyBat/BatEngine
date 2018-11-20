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

class ModelTestScene : public Bat::IScene
{
	DECLARE_CLASS( ModelTestScene, IScene );
public:
	ModelTestScene( Bat::Window& wnd );

	virtual void OnUpdate( float deltatime ) override;
	virtual void OnRender() override;
private:
	std::unique_ptr<Bat::IModel> m_pNanoSuit;
	Bat::MoveableCamera m_Camera;

	std::unique_ptr<DirectX::SpriteBatch> m_pSpriteBatch;
	std::unique_ptr<DirectX::SpriteFont> m_pFont;

	Bat::Light m_Light;

	float lightPos[3] = { 0.0f };
	float lightAmb[3] = { 0.2f };
	float lightDiff[3] = { 0.5f };
	float lightSpec[3] = { 1.0f };
};