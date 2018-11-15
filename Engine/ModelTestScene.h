#pragma once

#include "IScene.h"
#include "Texture.h"
#include "ClassDef.h"

namespace Bat
{
	class Window;
	class IModel;
}

class ModelTestScene : public Bat::IScene
{
	DECLARE_CLASS( ModelTestScene, IScene );
public:
	ModelTestScene( Bat::Window& wnd );

	virtual void OnUpdate( float deltatime ) override;
	virtual void OnRender() override;
private:
	Bat::IModel* pNanoSuit;
};