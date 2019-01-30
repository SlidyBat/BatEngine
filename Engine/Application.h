#pragma once

#include "ILayer.h"
#include "Mesh.h"
#include "Texture.h"

namespace Bat
{
	class Graphics;
	class Window;

	class Application : public ILayer
	{
	public:
		Application( Graphics& gfx, Window& wnd );
		~Application();

		virtual void OnUpdate( float deltatime ) override;
		virtual void OnRender() override;
	private:
		Graphics& gfx;
		Window& wnd;
		Texture screentex;
		Mesh screenquad;
		Colour* pixels = nullptr;

		// fps calculation
		int fps_counter = 0;
		float elapsed_time = 0.0f;
		std::string fps_string = "FPS: 0";
	};
}