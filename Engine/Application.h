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
#include "MouseEvents.h"
#include "PhysicsSystem.h"
#include "AnimationSystem.h"

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

		void OnEvent( const WindowResizeEvent& e );
		void OnEvent( const KeyPressedEvent& e );
		void OnEvent( const MouseButtonPressedEvent& e );
	private:
		void LoadModel( const std::string& filename );
		void BuildRenderGraph();
	public:
		bool opaque_pass = true;
		bool transparent_pass = true;

		bool bloom_enabled = true;
		bool motion_blur_enabled = false;
		bool tonemapping_enabled = true;
	private:
		Graphics& gfx;
		Window& wnd;
		ISoundEngine* snd;
		MoveableCamera camera;
		SceneNode scene;
		size_t scale_index;
		RenderGraph rendergraph;
		Entity flashlight;
		Entity sun;
		Entity player;

		bool physics_simulate = false;

		float timestamp = 0.0f;
		float anim_timescale = 1.0f;
		int selected_anim = 0;

		HierarchySystem hier_system;
		PhysicsSystem physics_system;
		AnimationSystem anim_system;

		float bloom_threshold = 1.0f;
		float exposure = 2.0f;

		bool imgui_menu_enabled = false;

		// FPS
		float elapsed_time = 0.0f;
		int fps_counter = 0;
		std::string fps_string;
	};
}