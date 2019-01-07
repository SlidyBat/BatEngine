#pragma once

#include "ILayer.h"
#include "Scene.h"
#include "MoveableCamera.h"
#include "MouseEvents.h"

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

		void OnFileChanged( const std::string& filename );
	private:
		Graphics& gfx;
		Window& wnd;
		SceneGraph scene;
		MoveableCamera camera;
		Light* light;
		Overlay* ui_overlay;

		// fps calculation
		int fps_counter = 0;
		float elapsed_time = 0.0f;
		std::string fps_string = "FPS: 0";
	};
}