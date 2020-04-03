#pragma once

#include <string>

namespace Bat
{
	class Graphics;
	class Window;
	class RenderGraph;
}

class RenderBuilder
{
public:
	void Make( const Bat::Graphics& gfx, const Bat::Window& wnd, Bat::RenderGraph* graph );
	void MakeForwardPipeline( const Bat::Graphics& gfx, const Bat::Window& wnd, Bat::RenderGraph* graph );

	void DrawSettings( const Bat::Graphics& gfx, const Bat::Window& wnd, Bat::RenderGraph* graph );
public:
	bool opaque_pass = true;
	bool transparent_pass = true;

	bool skybox_enabled = true;
	bool bloom_enabled = false;
	bool motion_blur_enabled = false;
	bool tonemapping_enabled = true;

	std::string skybox_tex = "Assets/Ignore/IBLTest.hdr";
	float bloom_threshold = 1.0f;
	float exposure = 2.0f;

	bool msaa_enabled = true;
	int msaa_samples = 4;
};