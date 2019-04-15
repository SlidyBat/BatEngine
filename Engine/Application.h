#pragma once

#include "ILayer.h"
#include "MoveableCamera.h"
#include "Mesh.h"
#include "Texture.h"
#include "Console.h"
#include "Audio.h"
#include "Scene.h"
#include "RenderGraph.h"

namespace Bat
{
	class Graphics;
	class Window;
	class IHost;
	class IPeer;
	template <class Application>
	class Scene;

	class Application : public ILayer
	{
	public:
		Application( Graphics& gfx, Window& wnd );
		~Application();

		virtual void OnUpdate( float deltatime ) override;
		virtual void OnRender() override;

		void BuildRenderGraph();
	public:
		bool bloom_enabled = true;
		bool motion_blur_enabled = true;
	private:
		Graphics& gfx;
		Window& wnd;
		ISoundEngine* snd;
		ISound* bell;
		MoveableCamera camera;
		SceneGraph scene;
		Light* light;
		Model* model;
		Overlay* overlay;
		RenderGraph rendergraph;

		// fps calculation
		int fps_counter = 0;
		float elapsed_time = 0.0f;
		std::string fps_string = "FPS: 0";
	};
}