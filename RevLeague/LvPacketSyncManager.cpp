#include "LvPacketSyncManager.h"

void LvPacketSyncManager::OnSentMovePacket(NetworkId movingUnit, int assignedSyncId)
{
	// todo: add queued packet
}

void LvPacketSyncManager::OnReceivedMovePacket(int syncId)
{
	// todo: remove from queued packets
}

void LvPacketSyncManager::Update()
{
	// evict all stale syncIDs for units that a newer syncIDs exists
	// mark timeouted (e.g. 3 seconds or preferably calculated from ping) packets for retransmission
}
