#pragma once

#include "NvLib.h"
#include "LvTypes.h"

#define MAKE_REPLICATION_ID(section, fieldId) ((section) << 8 | (fieldId))

enum class LvStatReplicationId : unsigned short {
	Hero_Gold = MAKE_REPLICATION_ID(0, 0),
	Hero_CanCastBits1 = MAKE_REPLICATION_ID(0, 1),
	Hero_CanCastBits2 = MAKE_REPLICATION_ID(0, 2),

	Hero_CurrentHealth = MAKE_REPLICATION_ID(3, 0),
	Hero_CurrentResource = MAKE_REPLICATION_ID(3, 1),
	Hero_MaxHealth = MAKE_REPLICATION_ID(3, 2),
	Hero_MaxResource = MAKE_REPLICATION_ID(3, 3),
	Hero_MovementSpeed = MAKE_REPLICATION_ID(3, 10),
	Hero_AttackRange = MAKE_REPLICATION_ID(1, 12),
	Hero_AttackSpeed = MAKE_REPLICATION_ID(1, 18),

	Hero_AttackDamageBase = MAKE_REPLICATION_ID(1, 4),
	Hero_AttackDamageBonus = MAKE_REPLICATION_ID(1, 13),

	Minion_CurrentHealth = MAKE_REPLICATION_ID(1, 0),
	Minion_MaxHealth = MAKE_REPLICATION_ID(1, 2),
	Minion_MovementSpeed = MAKE_REPLICATION_ID(3, 2),

	Turret_CurrentHealth = MAKE_REPLICATION_ID(1, 0),
	Turret_MaxHealth = MAKE_REPLICATION_ID(1, 1),
};

class LvStatReplicator : public NvNonCopyable {
private:
	LvStatsBase* associatedStats;
	unsigned int statCache[6][32] = { 0 };
	unsigned int statVisibilityStatus[6][32] = { 0 }; // if n-th bit is set to 1, then the stat will be replicated for player with index n

	void SetStatInCache(LvStatReplicationId statId, unsigned int value);
	bool ReplicateSection(LvPlayer* player, unsigned char* packetData, unsigned int section, unsigned int* bytesUsed);

public:
	LvStatReplicator(LvStatsBase* stats) : associatedStats(stats) {}

	void NotifyStatChange(LvStatReplicationId statId, float value) { this->SetStatInCache(statId, std::bit_cast<unsigned int>(value)); }
	void NotifyStatChange(LvStatReplicationId statId, unsigned int value) { this->SetStatInCache(statId, value); }

	NvBinaryStreamWrite ReplicateForPlayer(LvPlayer* player);
	NvBinaryStreamWrite CreateFakeReplicationPacket(unsigned char statSectionId, unsigned char statFieldId, unsigned int value);
};
