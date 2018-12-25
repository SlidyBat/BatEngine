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

		virtual void OnUpdate( float deltatime ) override;
		virtual void OnRender() override;

		void OnFileChanged( const std::string& filename );
		void OnEvent( const MouseButtonPressedEvent& e );
	private:
		Graphics& gfx;
		Window& wnd;
		SceneGraph scene;
		MoveableCamera camera;
		Light* light;

		// fps calculation
		int fps_counter = 0;
		float elapsed_time = 0.0f;
		std::string fps_string = "FPS: 0";
	};
}