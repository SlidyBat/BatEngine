#include "PCH.h"
#include "Networking.h"

#include <enet/enet.h>

namespace Bat
{
	static Networking::Event::Type ENet2BatEventType( int type )
	{
		switch( type )
		{
			case ENET_EVENT_TYPE_NONE:
				return Networking::Event::Type::NONE;
			case ENET_EVENT_TYPE_CONNECT:
				return Networking::Event::Type::CONNECT;
			case ENET_EVENT_TYPE_DISCONNECT:
				return Networking::Event::Type::DISCONNECT;
			case ENET_EVENT_TYPE_RECEIVE:
				return Networking::Event::Type::RECEIVE;
			default:
				ASSERT( false, "Unhandled ENet event type ({})", type );
				return Networking::Event::Type::NONE;
		}
	}

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

		virtual std::optional<Networking::Event> Service() override
		{
			ENetEvent enet_event;
			if( enet_host_service(m_pHost, &enet_event, 0) > 0 )
			{
				Networking::Event event;
				event.type = ENet2BatEventType( enet_event.type );
				event.peer = new ENPeer( enet_event.peer );
				if( enet_event.packet )
				{
					event.packet.data = (char*)enet_event.packet->data;
					event.packet.length = enet_event.packet->dataLength;
				}
				else
				{
					event.packet.data = nullptr;
					event.packet.length = 0;
				}

				return event;
			}

			return {};
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
}