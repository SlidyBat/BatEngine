#pragma once

#include "ILayer.h"
#include "MoveableCamera.h"
#include "Mesh.h"
#include "Texture.h"
#include "Console.h"
#include "Audio.h"
#include "Entity.h"
#include "RenderGraph.h"
#include "KeyboardEvents.h"

namespace Bat
{
	class Graphics;
	class Window;
	class Light;

	class Application : public ILayer
	{
	public:
		Application( Graphics& gfx, Window& wnd );
		~Application();

		virtual void OnUpdate( float deltatime ) override;
		virtual void OnRender() override;

		void OnEvent( const KeyPressedEvent& e );

		void BuildRenderGraph();
	public:
		bool bloom_enabled = true;
		bool motion_blur_enabled = false;
	private:
		Graphics& gfx;
		Window& wnd;
		ISoundEngine* snd;
		MoveableCamera* camera;
		SceneNode scene;
		RenderGraph rendergraph;
		Light* flashlight;
		Light* sun;

		float bloom_threshold = 1.0f;

		bool imgui_menu_enabled = false;

		// FPS
		float elapsed_time = 0.0f;
		int fps_counter = 0;
		std::string fps_string;
	};
}