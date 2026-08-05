#pragma once

#include "NvLib.h"
#include "LvTypes.h"

#include "Dependencies/enet/enet.h"

class LvClient final : public NvNonCopyable {
	struct LoadingProgress {
		float percentageLoaded = 0.0f;
		float eta = 1000.0f;
		unsigned int count = 0;
		unsigned int ping = 1000;
		bool isReady = false;

		unsigned int MakeExtraData() const
		{
			return (count & 0xffff) | ((ping & 0x7fff) << 16);
		}

		LoadingProgress() {}
		LoadingProgress(float percentageLoaded, float eta, unsigned int extraInfo, bool isReady) : percentageLoaded(percentageLoaded), eta(eta), isReady(isReady)
		{
			count = extraInfo & 0xffff;
			ping = (extraInfo >> 16) & 0x7fff;
		}
	};

	ENetPeer* peer;

	std::string endpointString;
	LvClientState state = CST_UNKNOWN;
	
	LvPlayer* associatedPlayer = nullptr;
	LoadingProgress loadingProgress;
	bool firstSelfLoadingPacketSent = false;

public:
	explicit LvClient(ENetPeer* peer);

	ENetPeer* GetENetPeer() const { return peer; }
	const std::string& GetEndpointString() const { return endpointString; }

	const LoadingProgress& GetLoadingProgress() const { return loadingProgress; }

	// all states equal to or greater than CST_REGISTERED imply that associatedPlayer is non-null
	LvClientState GetState() const { return state; }
	void SetState(LvClientState newState);

	LvPlayer* GetPlayer() const { return associatedPlayer; }
	void SetPlayer(LvPlayer* player); // unless player is nullptr, will also associate the player with this client

	void SendPacket(LvPacketChannel channelId, ENetPacket* packet, bool encryptPacket = true);
	void SendPacket(LvPacketChannel channelId, const NvBinaryStreamWrite& packet, unsigned int enetFlags = ENET_PACKET_FLAG_RELIABLE, bool encryptPacket = true);

	LvPacketProcessingResult ProcessPacket(LvPacketChannel channel, unsigned char* packetData, size_t packetSize);

	LvPacketProcessingResult HandleRegistration(NvBinaryStreamRead& stream);
	LvPacketProcessingResult HandleQueryStatus();
	LvPacketProcessingResult HandleSynchVersion(NvBinaryStreamRead& stream);
	LvPacketProcessingResult HandleCharSelected();
	LvPacketProcessingResult HandlePingLoadInfo(NvBinaryStreamRead& stream);
	LvPacketProcessingResult HandleClientReady();
};
