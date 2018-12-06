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
	std::unique_ptr<Bat::IModel> m_pModel;
	Bat::MoveableCamera m_Camera;

	Bat::Texture m_Skybox;

	Bat::Light m_Light;

	bool m_bUseBumpMap = false;

	float lightPos[3] = { 0.0f, 0.0f, -5.0f };
	float lightAmb[3] = { 0.2f, 0.2f, 0.2f };
	float lightDiff[3] = { 0.5f, 0.5f, 0.5f };
	float lightSpec[3] = { 1.0f, 1.0f, 1.0f };
};