#pragma once

#include "IScene.h"
#include "Texture.h"
#include "ClassDef.h"

class TestScene : public Bat::IScene
{
	DECLARE_CLASS( TestScene, IScene );
public:
	TestScene( Bat::Window& wnd, Bat::Graphics& gfx );

	virtual void OnUpdate() override;
	virtual void OnRender() override;
private:
	Bat::Model* pMario;
	Bat::Texture mariotex;
};