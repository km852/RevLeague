#pragma once

#include "NvLib.h"
#include "LvTypes.h"

#include "Dependencies/enet/enet.h"

class LvClient final : public NvNonCopyable {
	ENetPeer* peer;

	std::string endpointString;
	LvClientState state = CST_UNKNOWN;
	
	LvPlayer* associatedPlayer = nullptr;

public:
	explicit LvClient(ENetPeer* peer);

	ENetPeer* GetENetPeer() const { return peer; }
	const std::string& GetEndpointString() const { return endpointString; }

	// all states equal to or greater than CST_REGISTERED imply that associatedPlayer is non-null
	LvClientState GetState() const { return state; }
	void SetState(LvClientState newState);

	LvPlayer* GetPlayer() const { return associatedPlayer; }
	void SetPlayer(LvPlayer* player); // unless player is nullptr, will also associate the player with this client

	LvPacketProcessingResult ProcessPacket(LvPacketChannel channel, unsigned char* packetData, size_t packetSize);

	LvPacketProcessingResult HandleRegistration(NvBinaryStreamRead& stream);
	LvPacketProcessingResult HandleQueryStatus(NvBinaryStreamRead& stream);
};
