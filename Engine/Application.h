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
		bool bloom_enabled = false;
		bool motion_blur_enabled = false;
	private:
		Graphics& gfx;
		Window& wnd;
		ISoundEngine* snd;
		MoveableCamera camera;
		SceneGraph scene;
		RenderGraph rendergraph;

		float bloom_threshold = 1.0f;
	};
}