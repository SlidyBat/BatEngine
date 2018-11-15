#pragma once

#include "IScene.h"
#include "Texture.h"
#include "ClassDef.h"

class MarioTestScene : public Bat::IScene
{
	DECLARE_CLASS( MarioTestScene, IScene );
public:
	MarioTestScene( Bat::Window& wnd );

	virtual void OnUpdate() override;
	virtual void OnRender() override;
private:
	Bat::Model* pMario;
	Bat::Texture mariotex;
};