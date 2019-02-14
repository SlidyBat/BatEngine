#include "PCH.h"
#include "Networking.h"

#include <enet/enet.h>
#include <spdlog/fmt/fmt.h>
#include "NetworkEvents.h"

namespace Bat
{
	static ENetAddress Bat2ENetAddress( const Address& address )
	{
		ENetAddress enet_address;
		enet_address.host = address.host;
		enet_address.port = address.port;

		return enet_address;
	}

	static Address ENet2BatAddress( const ENetAddress& enet_address )
	{
		Address address;
		address.host = enet_address.host;
		address.port = enet_address.port;

		return address;
	}

	static Networking::PeerState ENet2BatPeerState( ENetPeerState state )
	{
		switch( state )
		{
			case ENET_PEER_STATE_DISCONNECTED:
				return Networking::PeerState::DISCONNECTED;
			case ENET_PEER_STATE_CONNECTING:
				return Networking::PeerState::CONNECTING;
			case ENET_PEER_STATE_ACKNOWLEDGING_CONNECT:
				return Networking::PeerState::ACKNOWLEDGING_CONNECT;
			case ENET_PEER_STATE_CONNECTION_PENDING:
				return Networking::PeerState::CONNECTION_PENDING;
			case ENET_PEER_STATE_CONNECTION_SUCCEEDED:
				return Networking::PeerState::CONNECTION_SUCCEEDED;
			case ENET_PEER_STATE_CONNECTED:
				return Networking::PeerState::CONNECTED;
			case ENET_PEER_STATE_DISCONNECT_LATER:
				return Networking::PeerState::DISCONNECT_LATER;
			case ENET_PEER_STATE_DISCONNECTING:
				return Networking::PeerState::DISCONNECTING;
			case ENET_PEER_STATE_ACKNOWLEDGING_DISCONNECT:
				return Networking::PeerState::ACKNOWLEDGING_DISCONNECT;
			case ENET_PEER_STATE_ZOMBIE:
				return Networking::PeerState::ZOMBIE;
			default:
				ASSERT( false, "Unhandled ENet peer state" );
				return Networking::PeerState::DISCONNECTED;
		}
	}

	class ENPeer : public IPeer
	{
	public:
		ENPeer( ENetPeer* pPeer )
			:
			m_pPeer( pPeer )
		{}

		virtual void Send( int channel, const Networking::Packet& packet ) override
		{
			ENetPacket* pPacket = enet_packet_create( packet.data,
				packet.length,
				ENET_PACKET_FLAG_RELIABLE
			);
			enet_peer_send( m_pPeer, (enet_uint8)channel, pPacket );
		}

		virtual void Disconnect() override
		{
			enet_peer_disconnect( m_pPeer, 0 );
		}

		virtual void Reset() override
		{
			enet_peer_reset( m_pPeer );
		}

		virtual Networking::PeerState GetState() const override
		{
			return ENet2BatPeerState( m_pPeer->state );
		}

		virtual Address GetAddress() const override
		{
			return ENet2BatAddress( m_pPeer->address );
		}

		virtual int GetPingInterval() const override
		{
			return (int)m_pPeer->pingInterval;
		}
	private:
		ENetPeer* m_pPeer;
	};

	class ENHost : public IHost
	{
	public:
		ENHost( ENetHost* pHost )
			:
			m_pHost( pHost )
		{}

		~ENHost()
		{
			enet_host_destroy( m_pHost );
		}

		virtual IPeer* Connect( const Address& address, int channel_count )
		{
			ENetAddress enet_address = Bat2ENetAddress( address );
			ENetPeer* pPeer = enet_host_connect( m_pHost,
				&enet_address,
				(size_t)channel_count,
				0
			);

			if( !pPeer )
			{
				return nullptr;
			}

			return new ENPeer( pPeer );
		}

		virtual void Service() override
		{
			ENetEvent enet_event;
			while( enet_host_service(m_pHost, &enet_event, 0) > 0 )
			{
				switch( enet_event.type )
				{
					case ENET_EVENT_TYPE_CONNECT:
					{
						ENPeer peer( enet_event.peer );
						DispatchEvent<PeerConnectedEvent>( this, &peer );
						break;
					}
					case ENET_EVENT_TYPE_DISCONNECT:
					{
						ENPeer peer( enet_event.peer );
						DispatchEvent<PeerDisconnectedEvent>( this, &peer );
						break;
					}
					case ENET_EVENT_TYPE_RECEIVE:
					{
						ENPeer peer( enet_event.peer );
						Networking::Packet packet;
						packet.data = reinterpret_cast<const char*>(enet_event.packet->data);
						packet.length = enet_event.packet->dataLength;
						DispatchEvent<PacketReceivedEvent>( this, &peer, packet, enet_event.channelID );
						break;
					}
					default:
					{
						ASSERT( false, "Unhandled ENet event type" );
						break;
					}
				}
			}
		}

		virtual void Broadcast( int channel, const Networking::Packet& packet ) override
		{
			ENetPacket* pPacket = enet_packet_create( packet.data,
				packet.length,
				ENET_PACKET_FLAG_RELIABLE
			);

			enet_host_broadcast( m_pHost, channel, pPacket );
		}
	private:
		ENetHost* m_pHost;
	};

	bool Networking::Initialize()
	{
		int ret = enet_initialize();
		ASSERT( ret >= 0, "Failed to initialize ENet" );

		return ret >= 0;
	}

	void Networking::Shutdown()
	{
		enet_deinitialize();
	}

	IHost* Networking::CreateClientHost( int max_out_connections, int max_channels, int max_in_bandwidth, int max_out_bandwidth )
	{
		ENetHost* pClient = enet_host_create( nullptr,
			(size_t)     max_out_connections,
			(size_t)     max_channels,
			(enet_uint32)max_in_bandwidth,
			(enet_uint32)max_out_bandwidth
		);

		return new ENHost( pClient );
	}

	IHost* Networking::CreateServerHost( const Address& address, int max_clients, int max_channels, int max_in_bandwidth, int max_out_bandwidth )
	{
		ENetAddress enet_address = Bat2ENetAddress( address );

		ENetHost* pServer = enet_host_create( &enet_address,
			(size_t)     max_clients,
			(size_t)     max_channels,
			(enet_uint32)max_in_bandwidth,
			(enet_uint32)max_out_bandwidth
		);

		return new ENHost( pServer );
	}

	void Networking::DestroyHost( IHost* pHost )
	{
		delete pHost;
	}

	Address Networking::CreateAddressFromHostname( const std::string& hostname, Port_t port )
	{
		ENetAddress address;
		address.port = port;
		enet_address_set_host( &address, hostname.data() );

		return { address.host, address.port };
	}

	Address Networking::CreateAddressFromIP( const std::string& ip, Port_t port )
	{
		ENetAddress address;
		address.port = port;
		enet_address_set_host( &address, ip.data() );

		return { address.host, address.port };
	}

	std::string Networking::GetHostnameFromAddress( const Address& address )
	{
		char buf[128];
		ENetAddress enet_address = Bat2ENetAddress( address );
		enet_address_get_host( &enet_address, buf, sizeof( buf ) );

		return buf;
	}

	std::string Networking::GetIPFromAddress( const Address& address )
	{
		char buf[128];
		ENetAddress enet_address = Bat2ENetAddress( address );
		enet_address_get_host_ip( &enet_address, buf, sizeof( buf ) );

		return buf;
	}

	std::string Address::ToString() const
	{
		std::string hostname = Networking::GetHostnameFromAddress( *this );
		return fmt::format( "{}:{}", hostname, port );
	}
}