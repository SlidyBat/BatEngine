#pragma once

#include "ILayer.h"
#include "Mesh.h"
#include "Texture.h"
#include "Console.h"

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
		void OnConnectCmd( const CommandArgs_t& args );
		void OnServerCmd( const CommandArgs_t& args );
		void OnSendCmd( const CommandArgs_t& args );
	private:
		Graphics& gfx;
		Window& wnd;
		IHost* client = nullptr;
		IHost* server = nullptr;
		std::vector<std::unique_ptr<IPeer>> peers;

		// fps calculation
		int fps_counter = 0;
		float elapsed_time = 0.0f;
		std::string fps_string = "FPS: 0";
	};
}