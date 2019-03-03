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

	class Application : public ILayer
	{
	public:
		Application( Graphics& gfx, Window& wnd );
		~Application();

		virtual void OnUpdate( float deltatime ) override;
		virtual void OnRender() override;
	private:
		void BuildRenderGraph();
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
		bool bloom_enabled = true;

		// fps calculation
		int fps_counter = 0;
		float elapsed_time = 0.0f;
		std::string fps_string = "FPS: 0";
	};
}