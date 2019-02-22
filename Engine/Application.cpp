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
#include "NetworkEvents.h"
#include "TexturePipeline.h"
#include "Globals.h"
#include "Networking.h"
#include <ntddkbd.h>

#include <imgui.h>

namespace Bat
{
	Application::Application( Graphics& gfx, Window& wnd )
		:
		gfx( gfx ),
		wnd( wnd )
	{
		client = Networking::CreateClientHost( 1, 1 );
		client->OnEventDispatched<PeerConnectedEvent>( []( const PeerConnectedEvent& e )
		{
			BAT_LOG( "[CLIENT] Peer '%s' connected", e.peer->GetAddress().ToString() );
		} );
		client->OnEventDispatched<PeerDisconnectedEvent>( []( const PeerDisconnectedEvent& e )
		{
			BAT_LOG( "[CLIENT] Peer '%s' disconnected", e.peer->GetAddress().ToString() );
		} );

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
			BAT_LOG( "You wrote: '%s'", Bat::JoinStrings( args ) );
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

		client->Service();
		if( server )
		{
			server->Service();
		}
	}

	void Application::OnRender()
	{
		gfx.DrawText( Bat::StringToWide( fps_string ).c_str(), DirectX::XMFLOAT2{ 15.0f, 15.0f } );
	}

	void Application::OnConnectCmd( const CommandArgs_t& args )
	{
		BAT_LOG( "Attempting to connect to '%s'...", args[1] );

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
		BAT_LOG( "Attempting to create server on '%s'...", args[1] );

		Address addr;
		addr.host = Networking::HOST_ANY;
		addr.port = std::stoi( std::string( args[1] ) );

		if( server ) Networking::DestroyHost( server );
		server = Networking::CreateServerHost( addr, 1, 1 );

		server->OnEventDispatched<PacketReceivedEvent>( []( const PacketReceivedEvent& e )
		{
			BAT_LOG( "[SERVER] Received packet from '%s'", e.peer->GetAddress().ToString() );

			std::string data;
			data.assign( e.packet.data, e.packet.length );
			BAT_LOG( "[SERVER] Packet data: %s", data );
		} );
	}

	void Application::OnSendCmd( const CommandArgs_t& args )
	{
		BAT_LOG( "Attempting to send '%s'...", args[1] );

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
