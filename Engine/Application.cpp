#include "PCH.h"
#include "Application.h"

#include "Common.h"
#include "Graphics.h"
#include "Window.h"
#include "SceneLoader.h"
#include "FileWatchdog.h"

#include "WindowEvents.h"
#include "KeyboardEvents.h"
#include "MouseEvents.h"
#include "TexturePipeline.h"
#include "Globals.h"
#include "Networking.h"

#include <imgui.h>

namespace Bat
{
	Application::Application( Graphics& gfx, Window& wnd )
		:
		gfx( gfx ),
		wnd( wnd )
	{
		client = Networking::CreateClientHost( 1, 1 );

		wnd.input.OnEventDispatched<KeyPressedEvent>( []( const KeyPressedEvent& e )
		{
			if( e.key == VK_OEM_3 )
			{
				BAT_LOG( "Toggling console" );
				g_Console.SetVisible( !g_Console.IsVisible() );
			}
		} );

		g_Console.AddCommand( "test_command", []( const CommandArgs_t& args )
		{
			BAT_LOG( "You wrote: '{}'", Bat::JoinStrings( args ) );
		} );

		g_Console.AddCommand( "connect", BIND_MEM_FN( Application::OnConnectCmd ) );
		g_Console.AddCommand( "create_server", BIND_MEM_FN( Application::OnServerCmd ) );
		g_Console.AddCommand( "send", BIND_MEM_FN( Application::OnSendCmd ) );
	}

	Application::~Application()
	{
		if( client ) Networking::DestroyHost( client );
		if( server ) Networking::DestroyHost( server );
	}

	void Application::OnUpdate( float deltatime )
	{
		elapsed_time += deltatime;
		fps_counter += 1;
		if( elapsed_time > 1.0f )
		{
			fps_string = "FPS: " + std::to_string( fps_counter );
			fps_counter = 0;
			elapsed_time -= 1.0f;
		}

		while( auto event = client->Service() )
		{
			switch( event->type )
			{
				case Networking::Event::Type::CONNECT:
				{
					BAT_LOG( "[CLIENT] Received CONNECT event" );
					break;
				}
				case Networking::Event::Type::DISCONNECT:
				{
					BAT_LOG( "[CLIENT] Received DISCONNECT event" );
					break;
				}
				case Networking::Event::Type::RECEIVE:
				{
					std::string data( event->packet.length, '\0' );
					memcpy( data.data(), event->packet.data, event->packet.length );
					BAT_LOG( "[CLIENT] Received packet data: '{}'", data );
					break;
				}
			}
			delete event->peer;
		}

		if( server )
		{
			while( auto event = server->Service() )
			{
				switch( event->type )
				{
					case Networking::Event::Type::CONNECT:
					{
						BAT_LOG( "[SERVER] Received CONNECT event" );
						break;
					}
					case Networking::Event::Type::DISCONNECT:
					{
						BAT_LOG( "[SERVER] Received DISCONNECT event" );
						break;
					}
					case Networking::Event::Type::RECEIVE:
					{
						std::string data( event->packet.length, '\0' );
						memcpy( data.data(), event->packet.data, event->packet.length );
						BAT_LOG( "[SERVER] Received packet data: '{}'", data );
						break;
					}
				}
				delete event->peer;
			}
		}
	}

	void Application::OnRender()
	{
		gfx.DrawText( Bat::StringToWide( fps_string ).c_str(), DirectX::XMFLOAT2{ 15.0f, 15.0f } );
	}

	void Application::OnConnectCmd( const CommandArgs_t& args )
	{
		BAT_LOG( "Attempting to connect to '{}'...", args[1] );

		auto strs = Bat::SplitString( args[1], ':' );

		Address addr;
		if( strs.size() > 1)
		{
			Port_t port = std::stoi( strs[1] );
			addr = Networking::CreateAddressFromHostname( strs[0], port );
		}
		else
		{
			addr = Networking::CreateAddressFromHostname( strs[0], 8000 );
		}

		IPeer* pPeer = client->Connect( addr, 1 );
		if( pPeer )
		{
			peers.emplace_back( pPeer );
		}
	}

	void Application::OnServerCmd( const CommandArgs_t& args )
	{
		BAT_LOG( "Attempting to create server on '{}'...", args[1] );

		auto strs = Bat::SplitString( args[1], ':' );

		Address addr;
		if( strs.size() > 1)
		{
			Port_t port = std::stoi( strs[1] );
			addr = Networking::CreateAddressFromHostname( strs[0], port );
		}
		else
		{
			addr = Networking::CreateAddressFromHostname( strs[0], 8000 );
		}

		server = Networking::CreateServerHost( addr, 1, 1 );
	}

	void Application::OnSendCmd( const CommandArgs_t& args )
	{
		BAT_LOG( "Attempting to send '{}'...", args[1] );

		if( !peers.size() )
		{
			BAT_ERROR( "No connected peers to send to!" );
		}

		Networking::Packet packet;
		packet.data = args[1].data();
		packet.length = args[1].length();
		client->Broadcast( 0, packet );
	}
}
