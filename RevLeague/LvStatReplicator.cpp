#include "LvStatReplicator.h"

#include "LvPlayer.h"
#include "LvObjectBase.h"
#include "LvStatsBase.h"

void LvStatReplicator::SetStatInCache(LvStatReplicationId statId, unsigned int value)
{
	unsigned short statValue = (unsigned short)statId;

	int statSection = (statValue >> 8) & 0xFF;
	int statFieldId = statValue & 0xFF;

	LogAssert(statSection <= 5);
	LogAssert(statFieldId <= 31);

	if (statSection > 5 || statFieldId > 31)
	{
		LvObjectBase* faultingObject = this->associatedStats->GetAssociatedObject();
		LogError("Attempted to fill an invalid stat {:04x} with value {:08x} for unit {}", statValue, value, faultingObject);
		return;
	}

	this->statCache[statSection][statFieldId] = value;
	this->statVisibilityStatus[statSection][statFieldId] = 0xFFFF;
}

bool LvStatReplicator::ReplicateSection(LvPlayer* player, unsigned char* packetData, unsigned int section, unsigned int* bytesUsed)
{
	unsigned int* packetIntData = reinterpret_cast<uint32_t*>(packetData);

	unsigned short visibilityMask = 1u << player->GetPlayerIndex();
	unsigned int fieldsToReplicate = 0;

	for (int i = 0; i < 32; ++i)
	{
		if (this->statVisibilityStatus[section][i] & visibilityMask)
		{
			fieldsToReplicate |= 1 << i;
			this->statVisibilityStatus[section][i] &= ~visibilityMask;
		}
	}

	if (fieldsToReplicate == 0)
		return false;

	memcpy(packetIntData++, &fieldsToReplicate, sizeof(unsigned int));

	for (int i = 0; i < 32; ++i)
	{
		if (fieldsToReplicate & (1 << i))
			memcpy(packetIntData++, &this->statCache[section][i], sizeof(unsigned int));
	}

	*bytesUsed += (unsigned int)((unsigned char*)packetIntData - packetData);
	return true;
}

NvBinaryStreamWrite LvStatReplicator::ReplicateForPlayer(LvPlayer* player)
{
	unsigned char replicationMask = 0;
	unsigned char replicationData[1024]; // will be enough to potentially store all data at once (max. 768 bytes)

	unsigned int bytesUsed = 0;

	for (int section : { 0, 1, 2, 3, 5, 4 }) // section order is: 1 2 4 8 32 16 (for some reason)
	{
		if (this->ReplicateSection(player, replicationData + bytesUsed, section, &bytesUsed))
			replicationMask |= 1 << section;
	}

	if (replicationMask == 0)
		return {};

	NvBinaryStreamWrite finalData(bytesUsed + 8);

	finalData.Write<unsigned char>(replicationMask);
	finalData.Write(this->associatedStats->GetAssociatedObject()->GetNetworkId());
	finalData.WriteBytes(replicationData, bytesUsed);

	return finalData;
}
