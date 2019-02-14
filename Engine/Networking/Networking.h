#pragma once

#include "Event.h"

namespace Bat
{
	class IHost;
	class IPeer;

	using Port_t = uint16_t;
	using Host_t = uint32_t;

	struct Address
	{
		Host_t host;
		Port_t port;

		std::string ToString() const;
	};

	class Networking
	{
	public:
		struct Packet
		{
			const char* data;
			size_t length;
		};

		enum class PeerState
		{
			DISCONNECTED,
			CONNECTING,
			ACKNOWLEDGING_CONNECT,
			CONNECTION_PENDING,
			CONNECTION_SUCCEEDED,
			CONNECTED,
			DISCONNECT_LATER,
			DISCONNECTING,
			ACKNOWLEDGING_DISCONNECT,
			ZOMBIE
		};

		static constexpr Host_t HOST_ANY = 0;
		static constexpr Port_t PORT_ANY = 0;

		static bool        Initialize();
		static void        Shutdown();

		static IHost* CreateClientHost(int max_out_connections,
			int max_channels,
			int max_in_bandwidth = 0,
			int max_out_bandwidth = 0
		);

		static IHost* CreateServerHost(const Address& address,
			int max_clients,
			int max_channels,
			int max_in_bandwidth = 0,
			int max_out_bandwidth = 0
		);
		static void        DestroyHost( IHost* pHost );

		static Address     CreateAddressFromHostname( const std::string& hostname, Port_t port );
		static Address     CreateAddressFromIP( const std::string& ip, Port_t port );
		static std::string GetHostnameFromAddress( const Address& address );
		static std::string GetIPFromAddress( const Address& address );
	};

	class IPeer
	{
	public:
		virtual ~IPeer() = default;

		// Sends a packet to the peer on the specified channel
		virtual void Send( int channel, const Networking::Packet& packet ) = 0;

		// Gently disconnect the peer by sending a disconnect request and waiting
		// for an acknowledgement. A DISCONNECT event will be generated once the
		// disconnection succeeds.
		virtual void Disconnect() = 0;

		// Forcefully disconnect the peer. The peer will not recieve any notification
		// of the disconnection and will time out. Additionally no DISCONNECT event
		// will be generated.
		virtual void Reset() = 0;

		virtual Networking::PeerState GetState() const = 0;
		virtual Address               GetAddress() const = 0;
		virtual int                   GetPingInterval() const = 0;
	};

	class IHost : public EventDispatcher
	{
	public:
		virtual ~IHost() = default;

		// Attempts to connect to the peer on the specified address. Once the
		// connection succeeds a CONNECT event will be generated.
		virtual IPeer* Connect( const Address& address, int channel_count ) = 0;

		// Services any incoming requests and dispatches events if there are any
		// See NetworkEvents.h for a list of events
		virtual void Service() = 0;
		
		// Broadcasts a packet to all connected peers
		virtual void Broadcast( int channel, const Networking::Packet& packet ) = 0;
	};
}