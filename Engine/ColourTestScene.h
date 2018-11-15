#pragma once

#include "IScene.h"
#include "Texture.h"
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

	virtual void OnUpdate() override;
	virtual void OnRender() override;
private:
	Bat::IModel* pRedSquare;
};