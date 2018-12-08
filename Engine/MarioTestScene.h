#pragma once

#include "IScene.h"
#include "ClassDef.h"
#include "MoveableCamera.h"

namespace Bat
{
	class Window;
	class IModel;
}

class MarioTestScene : public Bat::IScene
{
	DECLARE_CLASS( MarioTestScene, IScene );
public:
	MarioTestScene( Bat::Window& wnd );

	virtual void OnUpdate( float deltatime ) override;
	virtual void OnRender() override;
private:
	Bat::MoveableCamera m_Camera;
	std::unique_ptr<Bat::IModel> m_pMario;
};