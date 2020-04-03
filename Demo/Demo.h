#pragma once

#include "BatWinAPI.h"
#include "Application.h"
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
#include "CharacterControllerSystem.h"
#include "BehaviourTree.h"
#include "NavMesh.h"
#include "Particles.h"
#include "Scene.h"

namespace Bat
{
	class Graphics;
	class Window;
	class Light;
}

class Demo : public Bat::IApplication
{
public:
	Demo( Bat::Graphics& gfx, Bat::Window& wnd );
	~Demo();

	virtual void OnUpdate( float deltatime ) override;
	virtual void OnRender() override;

	void OnEvent( const Bat::WindowResizeEvent& e );
	void OnEvent( const Bat::KeyPressedEvent& e );
	void OnEvent( const Bat::MouseButtonPressedEvent& e );
private:
	void LoadModel( const std::string& filename );
	void BuildRenderGraph();
public:
	bool opaque_pass = true;
	bool transparent_pass = true;

	bool skybox_enabled = true;
	bool bloom_enabled = false;
	bool motion_blur_enabled = false;
	bool tonemapping_enabled = true;
private:
	Bat::Graphics& gfx;
	Bat::Window& wnd;
	Bat::MoveableCamera camera;
	Bat::SceneNode scene;
	Bat::SceneNode* scale_node;
	Bat::RenderGraph rendergraph;
	Bat::Entity flashlight;
	Bat::Entity sun;

	bool physics_simulate = true;
	bool draw_navmesh = false;

	float timestamp = 0.0f;
	float anim_timescale = 1.0f;
	int selected_anim = 0;

	Bat::HierarchySystem hier_system;
	Bat::PhysicsSystem physics_system;
	Bat::AnimationSystem anim_system;
	Bat::ParticleSystem particle_system;
	Bat::NavMeshSystem navmesh_system;
	Bat::BehaviourTreeSystem behaviour_system;
	Bat::CharacterControllerSystem controller_system;

	std::string skybox_tex = "Assets/Ignore/IBLTest.hdr";

	float bloom_threshold = 1.0f;
	float exposure = 2.0f;

	bool msaa_enabled = true;
	int msaa_samples = 4;

	bool imgui_menu_enabled = false;

	// FPS
	float elapsed_time = 0.0f;
	int fps_counter = 0;
	std::string fps_string;
};