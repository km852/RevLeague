#pragma once

#include "LvTypes.h"
#include "LvGameClock.h"

class LvPacketSyncManager final : public NvNonCopyable {
	struct MovePacketInfo {
		LvGameTimer sentTimer;
		NetworkId netId;
		int packetSyncId;
	};

	// we expect these vectors to be small, no need to introduce the overhead of a map
	std::vector<MovePacketInfo> pendingMovePackets;

public:
	void OnSentMovePacket(NetworkId movingUnit, int assignedSyncId); // "sent" = from server to client
	void OnReceivedMovePacket(int syncId); // "received" = from client to server

	void Update();
};
