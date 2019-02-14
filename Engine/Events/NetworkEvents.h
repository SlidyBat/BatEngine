#pragma once

#include "Networking.h"

namespace Bat
{
	// Called when a peer successfully connects
	// WARNING: `peer` pointer will be invalidated after event
	struct PeerConnectedEvent
	{
		// Host that peer connected to
		IHost* host;
		// Peer that was connected
		// WARNING: pointer will be invalidated after event
		IPeer* peer;
	};

	// Called when a peer successfully disconnects or when
	// a peer failed to initially connect
	// WARNING: `peer` pointer will be invalidated after event
	struct PeerDisconnectedEvent
	{
		// Host that peer disconnected from
		IHost* host;
		// Peer that was disconnected
		// WARNING: pointer will be invalidated after event
		IPeer* peer;
	};

	// Called whenever a packet is received
	// WARNING: `peer` pointer will be invalidated after event
	struct PacketReceivedEvent
	{
		// Host that packet was sent to
		IHost* host;
		// Peer that sent the packet
		// WARNING: pointer will be invalidated after event
		IPeer* peer;
		// Packet data itself
		// WARNING: packet data will be invalidated after event,
		//          copy locally if needed
		Networking::Packet packet;
		// Channel over which packet was received
		int channel;
	};
}