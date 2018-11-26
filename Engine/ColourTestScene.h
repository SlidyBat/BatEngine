#pragma once

#include "IScene.h"
#include "Texture.h"
#include "MoveableCamera.h"
#include "ClassDef.h"

namespace Bat
{
	class Window;
	class IModel;
}

class ColourTestScene : public Bat::IScene
{
	DECLARE_CLASS( ColourTestScene, IScene );
public:
	ColourTestScene( Bat::Window& wnd );

	virtual void OnUpdate( float deltatime ) override;
	virtual void OnRender() override;
private:
	Bat::MoveableCamera m_Camera;
	std::unique_ptr<Bat::IModel> m_pRedSquare;
};